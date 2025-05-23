"---START---"
C:/Users/knano/mywork/.venv/Scripts/activate.ps1
$ave_score = 0
$N = 100
for ($i = 0; $i -lt $N; $i++) {
    $filename = "in/{0:0000}.txt" -f $i
    $filename
    cat $filename | cargo run -r --bin tester python solve.py > out.txt
    $lines = Get-Content score.txt
    $now_score = [int]$lines
    $ave_score += $now_score
    if ($now_score -ge 1000000000) {
        Write-Host "NOT SOLVED" -ForegroundColor "Red"
    }
}
$ave_score = $ave_score / $N / 1000000
"---END---"

"RESULT: {0}" -f $ave_score