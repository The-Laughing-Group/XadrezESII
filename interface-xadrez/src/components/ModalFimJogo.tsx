import { useEffect } from "react";
import { Link } from "react-router-dom";

import casa from "../assets/icones/casa.png";
import reiniciar from "../assets/icones/reiniciar.png";

interface ModalFimJogoProps { 
    aberto: boolean;
    aoFechar: () => void;
    reiniciarJogo: () => void;
    textoIndicador: string;
};

const ModalConfiguracoes = ({ aberto, 
                            aoFechar, 
                            reiniciarJogo, 
                            textoIndicador }: ModalFimJogoProps) => {
    useEffect(() => {
        if(!aberto) return;
        const aoTeclar = (e: KeyboardEvent) => e.key === "Escape" && aoFechar();
        window.addEventListener("keydown", aoTeclar);
        return () => window.removeEventListener("keydown", aoTeclar);
    }, [aberto, aoFechar]);

    if(!aberto) return null;

    return (
        <div className="modal-fundo" onClick={aoFechar}>
            <div className="modal-caixa" role="dialog" aria-modal="true"
                 onClick={(e) => e.stopPropagation()}>
                <button className="modal-fechar" onClick={aoFechar}>⨉</button>
                <div className="modal-fim">
                    <h1>{textoIndicador}</h1>
                    <div>
                        <button className="botao-pixel" onClick={reiniciarJogo}><img className="icone-botao" src={reiniciar}/>Reiniciar</button>
                        <Link className="botao-pixel" to="/"><img className="icone-botao" src={casa}/>Início</Link>
                    </div>
                </div>
            </div>
        </div>
    );
};

export default ModalConfiguracoes;