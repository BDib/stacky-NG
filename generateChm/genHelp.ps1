# genHelp.ps1
# Usage: .\genHelp.ps1

$mdFiles = Get-ChildItem "Docs/*.md"
$compiler = "C:\Program Files (x86)\HTML Help Workshop\hhc.exe"

# 1. Convert Markdown to HTML
foreach ($md in $mdFiles) {
    $html = $md.FullName -replace '\.md$', '.html'
    pandoc $md.FullName -o $html --standalone --css=style.css
}

# 2. Generate .hhp and .hhc (Basic logic)
# Note: You can expand this to use a custom array for sorting
$hhp = @"
[OPTIONS]
Title=StackyNG Help
Default topic=Docs/README.html
Compiled file=help.chm
Contents file=help.hhc
[FILES]
$((Get-ChildItem "Docs/*.html" | ForEach-Object { "Docs\" + $_.Name }) -join "`n")
"@
$hhp | Out-File "help.hhp" -Encoding ascii

# 3. Compile
if (Test-Path $compiler) {
    Start-Process -FilePath $compiler -ArgumentList "help.hhp" -Wait
    Write-Host "Success: help.chm is ready."
} else {
    Write-Error "Compiler not found at $compiler"
}