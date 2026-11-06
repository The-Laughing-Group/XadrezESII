import type { Peca, TipoPeca } from "../interfaces/Peca";
import type { Partida } from "../hooks/usePartida";

import { posicaoToUci, uciToPosicao, type Posicao } from "../utils/notacao";

import Casa from "./Casa";

import p from "../assets/pecas/peaoPreto.png";
import r from "../assets/pecas/torrePreta.png";
import n from "../assets/pecas/cavaloPreto.png";
import b from "../assets/pecas/bispoPreto.png";
import q from "../assets/pecas/rainhaPreta.png";
import k from "../assets/pecas/reiPreto.png";

import P from "../assets/pecas/peaoBranco.png";
import R from "../assets/pecas/torreBranca.png";
import N from "../assets/pecas/cavaloBranco.png";
import B from "../assets/pecas/bispoBranco.png";
import Q from "../assets/pecas/rainhaBranca.png";
import K from "../assets/pecas/reiBranco.png";

import kPEmXeque from "../assets/pecas/kingBlackBlinkDanger.gif";
import kBEmXeque from "../assets/pecas/kingWhiteBlinkDanger.gif";

const criarPeca = (tipo: TipoPeca, emXeque = false): Peca => {

    const imagens = {p, r, n, b, q, k, P, R, N, B, Q, K};

    const imagens_xeque: Partial<Record<TipoPeca, string>> = {
        K: kBEmXeque,
        k: kPEmXeque,
    };

    return {
        tipo,
        img: (emXeque && imagens_xeque[tipo]) || imagens[tipo]
    };
};

const FEN_PARA_TIPO: Record<string, TipoPeca> = {
    p: "p", r: "r", n: "n", b: "b", q: "q", k: "k",
    P: "P", R: "R", N: "N", B: "B", Q: "Q", K: "K",
};

interface TabuleiroProps {
    partida: Partida;
    promocao: any;
    setPromocao: (p: any) => void;
    casaSelecionada: any;
    setCasaSelecionada: (c: any) => void;
    movimentosPossiveis: Posicao[];
    setMovimentosPossiveis: (m: Posicao[]) => void;
    limparSelecao: () => void;
    contraIA: boolean;
}

const Tabuleiro = ({ partida, 
                    promocao, 
                    setPromocao, 
                    casaSelecionada, 
                    setCasaSelecionada,
                    movimentosPossiveis, 
                    setMovimentosPossiveis, 
                    limparSelecao,
                    contraIA }: TabuleiroProps) => {

    const { tabuleiro, vezDasBrancas, situacao, lancesDe, tentarLance } = partida;

    const eReiEmXeque = (row: number, col: number): boolean => {
        if (!situacao.emXeque && situacao.estado !== "xeque_mate") return false;

        const rei = vezDasBrancas ? "K" : "k";
        return tabuleiro[row][col] === rei;
    };

    

    const casaEstaSelecionada = (row: number, col: number): boolean => {
        return casaSelecionada?.row === row && casaSelecionada?.col === col;
    };

    const eMovimentoPossivel = (row: number, col: number): boolean => {
        return movimentosPossiveis.some((posicao) => posicao.row === row && posicao.col === col);
    };

    const podeSelecionar = (row: number, col: number) => {
        if(situacao.terminou || (contraIA && !vezDasBrancas)) return false;
        const peca = tabuleiro[row]?.[col];
        if(peca === ".") return false;
        const branca = peca === peca?.toUpperCase();
        return vezDasBrancas ? branca : !branca;
    }

    const selecionarCasa = (row: number, col: number) => {
        if(promocao) return;

        if(casaEstaSelecionada(row, col)) {
            limparSelecao();
            return;
        };

        // Realiza movimento
        if(casaSelecionada && eMovimentoPossivel(row, col)) {
            const uci = posicaoToUci(casaSelecionada.row, casaSelecionada.col);
            const movimentos = lancesDe(uci).filter(movimento => movimento.slice(2, 4) === posicaoToUci(row, col));

            if(movimentos.some(movimento => movimento.length === 5)) {
                setPromocao({origem: { row: casaSelecionada.row, 
                                       col: casaSelecionada.col }, 
                            destino: { row: row, col: col }});
                return;
            }

            tentarLance(posicaoToUci(casaSelecionada.row, casaSelecionada.col) + posicaoToUci(row, col));
            limparSelecao();
            return;
        }

        if(podeSelecionar(row, col)) {
            setCasaSelecionada({ row, col });

            const uci = posicaoToUci(row, col);
            
            setMovimentosPossiveis(
                lancesDe(uci).map((possivelMovimento) =>
                    uciToPosicao(possivelMovimento)
                ) || []
            );
            setPromocao(null);
            return;
        }
    }

    return (
        <>
            <div className="tabuleiro">
                <div id="tabuleiro">
                    {tabuleiro.flatMap((linha, row) =>
                        Array.from(linha).map((tipo, col) => {
                            return (
                                <Casa
                                    key={`${row}-${col}`}
                                    row={row}
                                    col={col}
                                    peca={tipo === "." ? null : criarPeca(FEN_PARA_TIPO[tipo], eReiEmXeque(row, col))}
                                    selecionada={casaEstaSelecionada(row,col)}
                                    onSelecionar={selecionarCasa}
                                    movimentoPossivel={eMovimentoPossivel(row, col)}
                                    podeSelecionar={podeSelecionar(row,col)}
                                />
                            );
                        })
                    )}
                </div>
            </div>
        </>
    );
};

export default Tabuleiro;
