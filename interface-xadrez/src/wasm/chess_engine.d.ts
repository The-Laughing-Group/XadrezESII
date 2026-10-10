// Tipos do módulo gerado pelo Emscripten (chess_engine.js).
// Se você acrescentar funções em engine/bindings/bindings.cpp, declare-as aqui também.

export interface JogoWasm {
    carregarFen(fen: string): boolean
    fen(): string
    vezDasBrancas(): boolean
    tabuleiro(): string[]          // 8 textos de 8 caracteres, '.' = casa vazia
    lancesDe(casa: string): string[]   // ex.: lancesDe("e2") -> ["e2e3", "e2e4"]
    delete(): void                 // libera a memória do objeto C++
}

export interface ModuloXadrez {
    Jogo: new () => JogoWasm
}

declare function criarModulo(): Promise<ModuloXadrez>
export default criarModulo
