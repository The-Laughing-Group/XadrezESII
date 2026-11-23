# XadrezESII

## Documentos
- Slides
  - [Apresentação I](https://canva.link/bemqxswep8zpp1x)
  - [Apresentação II](https://canva.link/qr9ze169udly1or)
  - [Apresentação III](https://canva.link/hav51aonr9o1lmn)
- [Análise de Requisitos](https://docs.google.com/document/d/1Fe5bNVtEYjxyEXzZVik3johVUNSXykkBqUsAtUnU578/edit?usp=sharing)
- [Análise de Riscos](https://docs.google.com/document/d/1Cp6d9goywOjF5f3_6RJU2Wfpv-qGOxragN6FXJpj5MQ/edit?usp=sharing)
- [EAP e Cronograma de Gantt](https://miro.com/app/board/uXjVHvnTBlM=/?share_link_id=266751974007)
- [Análise de Valor Agregado](https://docs.google.com/spreadsheets/d/1YgpwxNxlImJMcc6Xr1GFhVQhDr9ImlwEJ7RtAn7dvt0/edit?usp=sharing)


## Comandos
- Compilar backend com emscripten (funções do back devem ser compiladas para .wasm para serem usadas pelo front):
  - Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
  - $env:EMSDK = "C:\emsdk"
  - .\build-wasm.ps1 
- Build com Cmake para testes:
  - cmake -S engine -B engine/build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
  - cmake --build engine/build
  - cmake --build engine/build --config Debug
  - ctest --test-dir engine/build --output-on-failure
- Rodando a interface (frontend):
  - cd interface-xadrez
  - npm i
  - npm run dev
- Rodando os testes relativos ao frontend:
  - cd interface-xadrez
  - npm run test
  - npm run coverage
    - Gera uma pasta "coverage". Dentro, abrir "index.html" para visualizar a cobertura de testes.
- Rodando testes da IA / Engine:
  ----- Roda o executável direto (mostra o log de cada teste) -----

  - .\engine\build\Debug\chess_tests.exe
  
  - .\engine\build\Debug\chess_tests.exe --gtest_list_tests
  
  - .\engine\build\Debug\chess_tests.exe --gtest_filter="IA_Mate1.*"
  
  - .\engine\build\Debug\chess_tests.exe --gtest_filter="IA_Captura.Cavalo_DamaVsTorre"
  
  - .\engine\build\Debug\chess_tests.exe --gtest_filter="IA_*:-IA_Performance.*"
 
  ----- Perft profundo (demora minutos) -----

  - .\engine\build\Debug\chess_tests.exe --gtest_also_run_disabled_tests --gtest_filter="*PerftProfundo*"
 
  ---------------- Rodar a Cobertura de Testes ------------------------------

  - OpenCppCoverage.exe --sources="$PWD\infra-estrutura" --sources="$PWD\ia" --sources="$PWD\engine\src" --excluded_sources="$PWD\engine\tests" --export_type=cobertura:coverage.cobertura --export_type=html:relatorio_cobertura -- .\engine\build\Debug\chess_tests.exe

## Direitos autorais
- [Peças e tabuleiro](https://deja-view.itch.io/chess-assets)
- [Asset para IA](https://opengameart.org/content/android-robot-animated)
- [Animação de fundo](https://anasabdin.com/#home)
- [Música](https://www.youtube.com/watch?v=rYXVAAL_1ss)
- [Sons](https://sounddino.com/pt/)
