export const posicaoToUci = (row: number, col: number):string => {
    const coluna = String.fromCharCode("a".charCodeAt(0) + col);
    const linha = 8 - row;
    return `${coluna}${linha}`;
};

export const uciToPosicao = (uci: string): Posicao => {
    const destino = uci.substring(2, 4);

    const col = destino.charCodeAt(0) - "a".charCodeAt(0);
    const row = 8 - Number(destino[1]);

    return { row, col };
};

export type Posicao = { 
    row: number; 
    col: number; 
};