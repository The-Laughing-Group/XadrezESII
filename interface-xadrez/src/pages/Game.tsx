import { useEffect, useRef, useState } from "react";
import { useSearchParams } from "react-router-dom";

import { usePartida, type Partida } from "../hooks/usePartida";
import { useSom } from "../hooks/useSom";
import { useMusica } from "../hooks/useMusica";

import { desambiguacao, uciToPosicao, type Posicao } from "../utils/notacao";

import Tabuleiro from "../components/Tabuleiro";
import HistoricoMovimentos from "../components/HistoricoMovimentos";
import BarraSuperior from "../components/BarraSuperior";
import ModalConfiguracoes from "../components/ModalConfiguracoes";
import ModalFimJogo from "../components/ModalFimJogo";

const textoDaBarra = ({ situacao, vezDasBrancas }: Pick<Partida, "situacao" | "vezDasBrancas">): string => {
    switch (situacao.estado) {
        case "xeque_mate":            return `Xeque-mate!\n Vencem as ${situacao.vencedor}`;
        case "afogamento":            return "Empate: afogamento";
        case "material_insuficiente": return "Empate: material insuficiente";
        case "regra_dos_50":          return "Empate: regra dos 50 lances";
        case "repeticao":             return "Empate: repetição de posição";
        default: {
            const vez = `Vez das ${vezDasBrancas ? "brancas" : "pretas"}`;
            return situacao.emXeque ? `Xeque! ${vez}` : vez;
        }
    }
};

const Game = () => {
    useMusica();
    
    const partida = usePartida();
    const { pronta, tabuleiro, situacao, vezDasBrancas, reiniciar } = partida;

    const tocarMovimento = useSom("movimento"); 
    const tocarXeque = useSom("xeque");
    const tocarFimJogo = useSom("fimjogo");

    const [searchParams] = useSearchParams();
    const modoIA = searchParams.get("modo") === "ia";
    
    let profundidade : number;
    if(modoIA) {
        switch(searchParams.get("dificuldade")) {
            case "facil": { profundidade = 2; break; }
            case "medio": { profundidade = 3; break; }
            case "dificil": { profundidade = 5; break; }
            default: { profundidade = 3; }
        }
    }
    const [configAberta, setConfigAberta] = useState(false);
    const [fimJogoAberto, setFimJogoAberto] = useState(false);

    const [historicoMovimentos, setHistoricoMovimentos] = useState<(string)[]>([]);

    const tabuleiroAntes = useRef(tabuleiro);

    useEffect(() => {
        if(situacao.emXeque && !situacao.terminou) tocarXeque();
    }, [situacao.emXeque]);

    useEffect(() => {
        const { uci, tipo, captura, promocao } = partida.ultimoLance;
        if (!uci) return;

        if (!situacao.emXeque && !situacao.terminou) {
            tocarMovimento();
        }

        let lance: string;

        if (tipo === "roque_rei") lance = "0-0";
        else if (tipo === "roque_dama") lance = "0-0-0";
        else {
            const destino = uci.slice(2, 4);
            const { row, col } = uciToPosicao(uci);
            const peca = tabuleiro[row][col];

            const ePeao = promocao ? true : peca.toUpperCase() === "P";
            const promo = promocao ? promocao.toUpperCase() : "";

            lance = ePeao
                ? `${captura ? uci[0] + "x" : ""}${destino}${promo}`
                : `${peca.toUpperCase()}${desambiguacao(tabuleiroAntes.current, uci)}${captura ? "x" : ""}${destino}`;
        }
        setHistoricoMovimentos((prev) => [...prev, lance]);
    }, [partida.ultimoLance]);

    useEffect(() => {
        tabuleiroAntes.current = tabuleiro;
    }, [tabuleiro]);
    
    useEffect(() => {
        if(situacao.terminou) {
            setFimJogoAberto(true);
            tocarFimJogo();
        }
    }, [situacao.terminou]);

    const [casaSelecionada, setCasaSelecionada] = useState<Posicao| null>(null);

    const [movimentosPossiveis, setMovimentosPossiveis] = useState<Posicao[]>([]);

    const [promocao, setPromocao] = useState<{origem: Posicao; destino: Posicao} | null>(null);

    const limparSelecao = () => {
        setCasaSelecionada(null); 
        setMovimentosPossiveis([]);
        setPromocao(null);
    };

    const reiniciarJogo = () => {
        reiniciar();
        setHistoricoMovimentos([]);
        setFimJogoAberto(false);
    };

    useEffect(() => {
        if(!modoIA ||
           !partida.pronta ||
           partida.vezDasBrancas ||
           partida.situacao.terminou ||
           !partida.fen) {
            return;
        }

        const temporizador = setTimeout(() => {
            partida.jogarIA(profundidade);
        }, 600);

        return () => clearTimeout(temporizador);
    }, [
        modoIA,
        partida.pronta,
        partida.vezDasBrancas,
        partida.situacao.terminou,
        partida.fen,
        partida.jogarIA,
    ]);

    if(!pronta) return null;
    
    return (
        <div id="game">
            <div className="painel">
                <BarraSuperior
                    setConfigAberta={setConfigAberta}
                    promocao={promocao}
                    limparSelecao={limparSelecao}
                    partida={partida}
                    textoIndicador={textoDaBarra({ situacao, vezDasBrancas })}
                />

                <Tabuleiro 
                    partida={partida}
                    promocao={promocao}
                    setPromocao={setPromocao}
                    casaSelecionada={casaSelecionada}
                    setCasaSelecionada={setCasaSelecionada}
                    movimentosPossiveis={movimentosPossiveis}
                    setMovimentosPossiveis={setMovimentosPossiveis}
                    limparSelecao={limparSelecao}
                    contraIA={modoIA}
                />
            </div>

            <HistoricoMovimentos historicoMovimentos={historicoMovimentos} />

            <ModalConfiguracoes 
                aberto={configAberta} 
                reiniciarJogo={reiniciarJogo}
                aoFechar={() => setConfigAberta(false)} 
            />
            <ModalFimJogo 
                aberto={fimJogoAberto} 
                aoFechar={() => setFimJogoAberto(false)}
                reiniciarJogo={reiniciarJogo}
                textoIndicador={textoDaBarra({ situacao, vezDasBrancas })}
            />
        </div>
    );
};

export default Game;