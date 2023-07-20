# Portable Mode

Browser Tamer fully supports "portable mode". However, you have to manually enable it as it is uncommon.

> **Portable mode**
>
> Assumes that application will be ran from a portable medium, such as USB stick, mapped drive, or anything removeable. In this mode, all the application files and configuration will be stored in the same place, so they can be moved between computers without loss of configuration.
>
{style="note"}

## Before you start

Make sure that:
- You have downloaded `.zip` from [GitHub Releases](https://github.com/aloneguid/bt/releases).
- You have extracted `bt.exe` into a dedicate folder on removeable media.

## Making it portable

Browser Tamer will check if `.portable` file exists in the same folder as `bt.exe` is located. And if it does,

<procedure title="Portable mode">
<step>
<code>config.ini</code> will be placed alongside `bt.exe`.
</step>
<step>
Rule Hit Log will be placed alongside `bt.exe` if turned on.
</step>
</procedure>

## Creating portable marker

You can create `.portable` file either manually in any editor, or using the following PowerShell snippet:

```bash
New-Item .portable -Type File
```
