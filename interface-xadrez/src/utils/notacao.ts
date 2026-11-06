import type { Partida } from "../hooks/usePartida";

type Tabuleiro = Partida["tabuleiro"];

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

const alcanca = (tab: Tabuleiro, de: Posicao, para: Posicao): boolean => {
    const tipo = tab[de.row][de.col].toLowerCase();
    const dr = para.row - de.row;
    const dc = para.col - de.col;

    if (tipo === "n") return Math.abs(dr) * Math.abs(dc) === 2;

    const diagonal = Math.abs(dr) === Math.abs(dc);
    const reta = dr === 0 || dc === 0;
    const anda = tipo === "b" ? diagonal
               : tipo === "r" ? reta
               : tipo === "q" ? diagonal || reta
               : false;
    if (!anda) return false;

    const passoLinha = Math.sign(dr);
    const passoColuna = Math.sign(dc);
    let row = de.row + passoLinha;
    let col = de.col + passoColuna;

    while (row !== para.row || col !== para.col) {
        if (tab[row][col] !== ".") return false;
        row += passoLinha;
        col += passoColuna;
    }
    return true;
};

export const desambiguacao = (tabuleiroAntes: Tabuleiro, uci: string): string => {
    const origem: Posicao = { col: uci.charCodeAt(0) - 97, row: 8 - Number(uci[1]) };
    const destino = uciToPosicao(uci);

    const peca = tabuleiroAntes[origem.row][origem.col];
    const tipo = peca.toLowerCase();
    if (tipo === "p" || tipo === "k") return "";   

    const rivais: Posicao[] = [];
    for (let row = 0; row < 8; row++) {
        for (let col = 0; col < 8; col++) {
            if (row === origem.row && col === origem.col) continue;
            if (tabuleiroAntes[row][col] !== peca) continue;
            if (alcanca(tabuleiroAntes, { row, col }, destino)) rivais.push({ row, col });
        }
    }
    if (rivais.length === 0) return "";

    const coluna = String.fromCharCode(97 + origem.col);
    const linha = String(8 - origem.row);

    if (!rivais.some((r) => r.col === origem.col)) return coluna;   
    if (!rivais.some((r) => r.row === origem.row)) return linha;    
    return coluna + linha;                                          
};