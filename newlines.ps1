# Define the directory to search
$directory = "C:\Users\Matt\Projects\triton"

# Get all .cxx and .hpp files recursively
Get-ChildItem -Path $directory -Recurse -Include *.cxx, *.hpp | ForEach-Object {
    $file = $_.FullName

    # Read the file content
    $content = Get-Content $file -Raw

    # Check if the file ends with a newline
    if ($content -notmatch "\n$") {
        # Append a newline and write back to the file
        Set-Content -Path $file -Value ($content + "`n")
        Write-Host "Added newline to: $file"
    } else {
        Write-Host "Already ends with a newline: $file"
    }
}
