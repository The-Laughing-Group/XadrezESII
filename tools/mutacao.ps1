# tools/mutacao.ps1  -  Teste de mutacao no codigo-fonte REAL.
# Rode a partir da pasta XadrezESII:   .\tools\mutacao.ps1
# Para cada mutante: altera 1 trecho, recompila, roda os testes filtrados e RESTAURA o arquivo.
#   MORTO      = algum teste falhou/travou/abortou  (bom: o teste "apitou vermelho")
#   SOBREVIVEU = todos passaram                      (ruim: falta teste, ou mutante equivalente)
param([string]$Build = "engine/build", [string]$Config = "Debug", [int]$TimeoutSeg = 180)
$ErrorActionPreference = "Stop"
$exe = (Join-Path (Resolve-Path $Build) "$Config\chess_tests.exe")

$TT   = "CC_TT.*:Mutacao_*:Integracao_Inter.*"
$BUS  = "CC_Busca.*:Integracao_Inter.*:IA_Mate1.*"
$LAN  = "*CC_Lances*:Integracao_Intra.*:AplicarLance.*"
$JOGO = "CC_Jogo.*:Situacao.*:UltimoLance.*"

$mutantes = @(
 @{Id="M01"; Arq="infra-estrutura/otimizacao/tt.cpp"; De="pontuacao_tt >= beta"; Para="pontuacao_tt > beta"; F=$TT},
 @{Id="M02"; Arq="infra-estrutura/otimizacao/tt.cpp"; De="pontuacao_tt <= alpha"; Para="pontuacao_tt < alpha"; F=$TT},
 @{Id="M03"; Arq="infra-estrutura/otimizacao/tt.cpp"; De="profundidade <= entrada_tt.profundidade"; Para="profundidade < entrada_tt.profundidade"; F=$TT},
 @{Id="M04"; Arq="infra-estrutura/otimizacao/tt.cpp"; De="entrada_tt.flag == TTFlag::EXACT"; Para="entrada_tt.flag != TTFlag::EXACT"; F=$TT},
 @{Id="M05"; Arq="infra-estrutura/otimizacao/tt.cpp"; De="(profundidade >= entrada_tt.profundidade);"; Para="(profundidade > entrada_tt.profundidade);"; F=$TT},
 @{Id="M06"; Arq="infra-estrutura/otimizacao/tt.cpp"; De="(entrada_tt.idade != idade_atual)"; Para="(entrada_tt.idade == idade_atual)"; F=$TT},
 @{Id="M07"; Arq="infra-estrutura/otimizacao/tt.cpp"; De="entrada_tt.chave == chave && melhor_lance == SEM_LANCE"; Para="entrada_tt.chave == chave && melhor_lance != SEM_LANCE"; F=$TT},
 @{Id="M08"; Arq="infra-estrutura/otimizacao/tt.hpp"; De="if (pontuacao >=  SCORE_MATE_MIN) return pontuacao + ply;"; Para="if (pontuacao >=  SCORE_MATE_MIN) return pontuacao - ply;"; F=$TT},
 @{Id="M09"; Arq="infra-estrutura/otimizacao/tt.hpp"; De="if (pontuacao <= -SCORE_MATE_MIN) return pontuacao - ply;"; Para="if (pontuacao <= -SCORE_MATE_MIN) return pontuacao + ply;"; F=$TT},
 @{Id="M10"; Arq="ia/busca-minimax/search.cpp"; De="Cor::BRANCO ? -SCORE_MATE + ply"; Para="Cor::BRANCO ? -SCORE_MATE - ply"; F=$BUS},
 @{Id="M11"; Arq="ia/busca-minimax/search.cpp"; De="if (repetida(tabuleiro) || empate_50(tabuleiro)) return 0;"; Para="if (empate_50(tabuleiro)) return 0;"; F=$BUS},
 @{Id="M12"; Arq="ia/busca-minimax/search.cpp"; De="if (legais == 0)"; Para="if (legais < 0)"; F=$BUS},
 @{Id="M13"; Arq="ia/busca-minimax/search.cpp"; De="if (brancas ? valor > melhor_valor : valor < melhor_valor) {"; Para="if (brancas ? valor < melhor_valor : valor > melhor_valor) {"; F=$BUS},
 @{Id="M14"; Arq="ia/busca-minimax/search.cpp"; De="if (alfa >= beta) break;"; Para="if (alfa > beta) break;"; F=$BUS; Nota="equivalente provavel (so perde poda)"},
 @{Id="M15"; Arq="ia/busca-minimax/search.cpp"; De="melhor_valor <= alfa_original"; Para="melhor_valor < alfa_original"; F=$BUS; Nota="candidato a sobrevivente: pede teste de flag UPPERBOUND na fronteira"},
 @{Id="M16"; Arq="infra-estrutura/lances.cpp"; De="b.estado.contador_50++;"; Para="b.estado.contador_50 += 2;"; F=$LAN},
 @{Id="M17"; Arq="infra-estrutura/lances.cpp"; De="tpeca == TipoPeca::PEAO || eh_captura(m)"; Para="tpeca == TipoPeca::PEAO"; F=$LAN},
 @{Id="M18"; Arq="infra-estrutura/lances.cpp"; De="destino-8"; Para="destino+8"; F=$LAN},
 @{Id="M19"; Arq="infra-estrutura/lances.cpp"; De="r &= ~ROQUE_BRANCO_REI;"; Para="r &= ~ROQUE_BRANCO_DAMA;"; F=$LAN},
 @{Id="M20"; Arq="infra-estrutura/lances.cpp"; De="if (em_xeque(b, cor_atual)) {"; Para="if (em_xeque(b, cor_oponente)) {"; F=$LAN},
 @{Id="M21"; Arq="infra-estrutura/lances.cpp"; De="chave ^= ZOBRIST[ZOB_LADO];"; Para="chave ^= 0;"; F=$LAN},
 @{Id="M22"; Arq="engine/src/game/jogo.cpp"; De="lado_a_jogar != Cor::PRETO"; Para="lado_a_jogar == Cor::PRETO"; F=$JOGO},
 @{Id="M23"; Arq="engine/src/game/jogo.cpp"; De="contar_repeticoes(b) >= 2"; Para="contar_repeticoes(b) >= 1"; F=$JOGO},
 @{Id="M24"; Arq="engine/src/game/jogo.cpp"; De="(flags_do_lance(m) == LANCE_ROQUE_REI)"; Para="(flags_do_lance(m) != LANCE_ROQUE_REI)"; F=$JOGO},
 @{Id="M25"; Arq="engine/src/game/jogo.cpp"; De="? destino - 8 : destino + 8"; Para="? destino + 8 : destino - 8"; F=$JOGO}
)

$res = @()

# Antes do foreach: linha de base
foreach ($f in @($TT,$BUS,$LAN,$JOGO)) {
  & $exe "--gtest_filter=$f" *> $null
  if ($LASTEXITCODE -ne 0) { Write-Warning "LINHA DE BASE FALHA no filtro '$f'. Corrija antes de mutar."; }
}

foreach ($m in $mutantes) {
  if (-not (Test-Path $m.Arq)) { $res += [pscustomobject]@{Id=$m.Id; Status="ARQUIVO NAO ENCONTRADO (ajuste o caminho)"; Nota=$m.Arq}; continue }
  $caminho = (Resolve-Path $m.Arq).Path
  $bytes   = [System.IO.File]::ReadAllBytes($caminho)          # backup exato (BOM/CRLF)
  $texto   = [System.Text.Encoding]::UTF8.GetString($bytes)
  if (-not $texto.Contains($m.De)) { $res += [pscustomobject]@{Id=$m.Id; Status="TRECHO NAO ENCONTRADO"; Nota=$m.De}; continue }
  $status = ""
  try {
    $novo = $texto.Replace($m.De, $m.Para)
    [System.IO.File]::WriteAllBytes($caminho, (New-Object System.Text.UTF8Encoding($false)).GetBytes($novo))
    cmake --build $Build --config $Config 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) { $status = "NAO COMPILOU" }
    else {
      $out = "$env:TEMP\mut_out.txt"
      $p = Start-Process -FilePath $exe -ArgumentList "--gtest_filter=$($m.F)" -NoNewWindow -PassThru -RedirectStandardOutput $out
      $null = $p.Handle                       # garante que ExitCode não venha nulo
      if (-not $p.WaitForExit($TimeoutSeg * 1000)) {
        $p.Kill(); $status = "MORTO (timeout)"; $quem = "timeout"
      }
      elseif ($p.ExitCode -ne 0) {
        $status = "MORTO"
        $f = Select-String -Path $out -Pattern "\[  FAILED  \] \S+\.\S+" | Select-Object -First 1
        $quem = if ($f) { $f.Matches[0].Value } else { "abortou/crash (sem linha FAILED)" }
      }
      else { $status = "SOBREVIVEU"; $quem = "" }
      $m.Nota = "$($m.Nota) $quem".Trim()
    }
  } finally {
    [System.IO.File]::WriteAllBytes($caminho, $bytes)            # sempre restaura
  }
  $res += [pscustomobject]@{Id=$m.Id; Status=$status; Nota=$m.Nota}
}
cmake --build $Build --config $Config 2>&1 | Out-Null            # recompila o codigo original
$res | Format-Table -AutoSize
$mortos = ($res | Where-Object { $_.Status -like "MORTO*" }).Count
$validos = ($res | Where-Object { $_.Status -in @("MORTO","MORTO (timeout)","SOBREVIVEU") }).Count
if ($validos -gt 0) { "Mutation score: {0}/{1} = {2:P0}" -f $mortos, $validos, ($mortos / $validos) }