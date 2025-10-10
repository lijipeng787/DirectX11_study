# 1. Find and store all target source files
$FilesToFormat = Get-ChildItem -Path * -Recurse -Include *.cpp,*.h,*.hpp

# 2. Calculate total count and initialize counter
$TotalCount = $FilesToFormat.Count
$Counter = 0

# 3. Iterate through files and display progress
$FilesToFormat | ForEach-Object {
    # Increment counter
    $Counter++

    # Compute percent complete
    $PercentComplete = ($Counter / $TotalCount) * 100

    # Show progress bar
    Write-Progress -Activity "Clang-Format Formatting Source Files" `
                   -Status "Formatting file ($Counter / $TotalCount): $($_.Name)" `
                   -PercentComplete $PercentComplete

    # Run clang-format in-place
    clang-format -i $_.FullName
}

# 4. Clear the progress bar
Write-Progress -Activity "Clang-Format Formatting Source Files" -Completed $true