# XadrezESII

## Documentos
- [Slides](https://canva.link/bemqxswep8zpp1x)
- [Análise de Requisitos](https://docs.google.com/document/d/1Fe5bNVtEYjxyEXzZVik3johVUNSXykkBqUsAtUnU578/edit?usp=sharing)
- [Análise de Riscos](https://docs.google.com/document/d/1Cp6d9goywOjF5f3_6RJU2Wfpv-qGOxragN6FXJpj5MQ/edit?usp=sharing)
- [EAP e Cronograma de Gantt](https://miro.com/app/board/uXjVHvnTBlM=/?share_link_id=266751974007)
- source ./emsdk_env.sh

./scripts/engine.sh                    # compila e roda todos os testes
./scripts/engine.sh build              # só compila
./scripts/engine.sh test --filter Perft  # só os testes de perft
./scripts/engine.sh rebuild --release  # apaga o build e refaz em Release
./scripts/engine.sh clean              # apaga engine/build