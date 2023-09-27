import rich.progress
import os
import argparse
import glob
import hashlib
from rich.progress import Progress
from vt.client import Client
from vt.error import APIError
from dataclasses import dataclass

@dataclass
class ScanResult:
    file_path: str
    file_name: str
    sha256: str
    web_url: str

def scan_file(vt: Client, path: str, progress: rich.progress.Progress) -> ScanResult:
    with open(path, "rb", buffering=0) as f:
        sha256 = hashlib.file_digest(f, "sha256").hexdigest()
        progress.console.print(sha256)

        # get info or scan
        try:
            vt.get_object(f"/files/{sha256}")
        except APIError:
            vt.scan_file(f)

    return ScanResult(path,
                      os.path.split(path)[1],
                      sha256,
                      f"https://www.virustotal.com/gui/file/{sha256}?nocache=1")


if __name__ == "__main__":
    # parse input args
    parser = argparse.ArgumentParser("VT Scanner")
    parser.add_argument("--files", nargs="*")
    parser.add_argument("--md-output")
    args, _ = parser.parse_known_args()
    input_file_globs = args.files
    input_files = []
    for ifg in input_file_globs:
        for input_file in glob.glob(ifg):
            input_files.append(input_file)

    print(f"files to scan: {input_files}")

    api_key = os.getenv("VT_API_KEY")
    client = Client(api_key)
    results = []

    with Progress() as progress:
        file_task = progress.add_task("Processing files...", total=len(input_files))
        for input_file in input_files:
            results.append(scan_file(client, input_file, progress))
            progress.update(file_task, advance=1)

    # generate markdown
    if args.md_output:
        smd = ["## 🛡️ VirusTotal Analysis", "" ]
        for result in results:
            smd.append(f"\n - [{result.file_name}]({result.web_url})")

        with open(args.md_output, "w", encoding="utf-8") as f:
            f.writelines(smd)

    client.close()
