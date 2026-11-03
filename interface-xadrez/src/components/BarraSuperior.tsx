import { Link } from "react-router-dom";

import { useSom } from "../hooks/useSom";

import { posicaoToUci } from "../utils/notacao";

import config from "../assets/icones/config.png";
import casa from "../assets/icones/casa.png";

const OPCOES_DE_PROMOCAO = [
    ["q", "♛"],
    ["r", "♜"],
    ["b", "♝"],
    ["n", "♞"],
] as const;

interface BarraSuperiorProps {
    setConfigAberta: (aberto: boolean) => void;
    promocao: any;
    limparSelecao: () => void;
    partida: any;
    textoIndicador: string;
}

const BarraSuperior = ({ setConfigAberta,
                        promocao,
                        limparSelecao,
                        partida,
                        textoIndicador }: BarraSuperiorProps) => {

    const { vezDasBrancas, tentarLance } = partida;

    const tocarPromocao = useSom("promocao"); 

    const escolherPromocao = (letra: string) => {
        if (!promocao) return;

        if(tentarLance(posicaoToUci(promocao.origem.row, 
                                 promocao.origem.col) + 
                       posicaoToUci(promocao.destino.row, 
                                    promocao.destino.col) + 
                       letra)) {
            tocarPromocao();
        }
        limparSelecao();
    };

    return (
        <div className="barra-superior">
            <div className="barra-esq">
                <Link className="botao-pixel botao-icone" to="/"
                    aria-label="Casa" title="Casa">
                    <img src={casa} className="icone-botao" alt="" draggable={false} />
                </Link>
                <button className="botao-pixel"
                    onClick={() => setConfigAberta(true)}
                    aria-label="Configurações" title="Configurações">
                    <img src={config} className="icone-botao" alt="" draggable={false} />
                </button>
                {/* <div className={`indicador-vez ${vezDasBrancas ? "indicador-vez-branca" : "indicador-vez-preta"}`}>{textoDaBarra({situacao, vezDasBrancas})}</div> */}
                <div className={`indicador-vez ${vezDasBrancas ? "indicador-vez-branca" : "indicador-vez-preta"}`}>{textoIndicador}</div>
            </div>
            <div className="promocao">
                { promocao && (
                        <>
                            {OPCOES_DE_PROMOCAO.map(([tipo, nome]) => (
                                <button className="botao-pixel" key={tipo} onClick={() => escolherPromocao(tipo)}>
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