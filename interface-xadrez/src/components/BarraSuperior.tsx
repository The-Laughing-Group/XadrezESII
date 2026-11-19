import { useSom } from "../hooks/useSom";

import { posicaoToUci, type Posicao } from "../utils/notacao";

import config from "../assets/icones/config.png";
import type { Partida } from "../hooks/usePartida";

const OPCOES_DE_PROMOCAO = [
    ["q", "♛"],
    ["r", "♜"],
    ["b", "♝"],
    ["n", "♞"],
] as const;

interface BarraSuperiorProps {
    setConfigAberta: (aberto: boolean) => void;
    promocao: {origem: Posicao; destino: Posicao} | null;
    limparSelecao: () => void;
    partida: Partida;
    textoIndicador: string;
}

const BarraSuperior = ({ setConfigAberta,
                        promocao,
                        limparSelecao,
                        partida,
                        textoIndicador }: BarraSuperiorProps) => {

    const { vezDasBrancas, tentarLance } = partida;

    const tocarPromocao = useSom("promocao"); 
    
    const escolherPromocao = (
        { origem, destino }: { origem: Posicao; destino: Posicao },
        letra: string
    ) => {
        if (tentarLance(posicaoToUci(origem.row, origem.col) +
                        posicaoToUci(destino.row, destino.col) +
                        letra)) {
            tocarPromocao();
        }
        limparSelecao();
    };

    return (
        <div className="barra-superior">
            <div className="barra-esq">
                <button className="botao-pixel"
                    onClick={() => setConfigAberta(true)}
                    aria-label="Configurações" title="Configurações">
                    <img src={config} className="icone-botao" alt="" draggable={false} />
                </button>
                <div className={`indicador-vez ${vezDasBrancas ? "indicador-vez-branca" : "indicador-vez-preta"}`}>{textoIndicador}</div>
            </div>
            <div className="promocao">
                { promocao && (
                        <>
                            {OPCOES_DE_PROMOCAO.map(([tipo, nome]) => (
                                <button className="botao-pixel" key={tipo} onClick={() => escolherPromocao(promocao, tipo)}>
                                    {nome}
                                </button>
                            ))}
                            <button className="botao-pixel cancelar" onClick={limparSelecao}>⨉</button>
                        </>
                )}
            </div>   
        </div>
    )
}
export default BarraSuperior