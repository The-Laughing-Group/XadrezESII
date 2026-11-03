// Tipos do módulo gerado pelo Emscripten (chess_engine.js).
// Se você acrescentar funções em engine/bindings/bindings.cpp, declare-as aqui também.

// "O que aconteceu no último lance" (struct InfoLance do C++).
export type TipoDeLance =
  | "" // ainda não houve lance
  | "normal"
  | "avanco_duplo"
  | "captura"
  | "roque_rei"
  | "roque_dama"
  | "en_passant"
  | "promocao"
  | "captura_promocao";

export interface InfoLance {
  uci: string; // "e2e4"; '' se ainda não houve lance
  tipo: TipoDeLance;
  captura: boolean;
  pecaCapturada: string; // mesmo caractere de tabuleiro(): 'p', 'N'...; '' se não houve captura
  casaCapturada: string; // casa da peça removida. No en passant NÃO é a casa de destino!
  promocao: string; // 'q' | 'r' | 'b' | 'n', ou ''
  torreOrigem: string; // só no roque: "h1", "a8"...
  torreDestino: string; // só no roque: "f1", "d8"...
}

// "Como está a partida agora" (struct SituacaoPartida do C++).
export type EstadoDoJogo =
  | "em_andamento"
  | "xeque_mate"
  | "afogamento"
  | "material_insuficiente"
  | "regra_dos_50"
  | "repeticao";

export interface SituacaoPartida {
  estado: EstadoDoJogo;
  vencedor: "" | "brancas" | "pretas"; // só no xeque-mate
  emXeque: boolean; // o rei de quem tem a vez está em xeque
  terminou: boolean; // true para qualquer estado diferente de 'em_andamento'
}

export interface JogoWasm {
  carregarFen(fen: string): boolean;
  fen(): string;
  vezDasBrancas(): boolean;
  tabuleiro(): string[]; // 8 textos de 8 caracteres, '.' = casa vazia
  lancesDe(casa: string): string[]; // ex.: lancesDe("e2") -> ["e2e3", "e2e4"]
  tentarLance(uci: string): boolean; // ex.: tentarLance("e2e4"); true se o motor aceitou e atualizou
  emXeque(): boolean;
  emXequeMate(): boolean;
  emAfogamento(): boolean;
  situacao(): SituacaoPartida; // objeto JS comum (cópia): não precisa de delete()
  ultimoLance(): InfoLance; // idem
  delete(): void; // libera a memória do objeto C++
  melhorLanceIA(profundidade: number): string;
}

export interface ModuloXadrez {
  Jogo: new () => JogoWasm;
}

declare function criarModulo(): Promise<ModuloXadrez>;
export default criarModulo;
