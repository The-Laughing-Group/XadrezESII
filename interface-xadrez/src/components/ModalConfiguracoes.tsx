import { useEffect } from "react";
import PainelConfiguracoes from "./PainelConfiguracoes";
import { Link } from "react-router-dom";

import casa from "../assets/icones/casa.png";
import reiniciar from "../assets/icones/reiniciar.png";

interface ModalConfiguracoesProps { 
    aberto: boolean;
    reiniciarJogo: () => void;
    aoFechar: () => void;
};

const ModalConfiguracoes = ({ aberto, reiniciarJogo, aoFechar }: ModalConfiguracoesProps) => {
    useEffect(() => {
        if (!aberto) return;
        const aoTeclar = (e: KeyboardEvent) => e.key === "Escape" && aoFechar();
        window.addEventListener("keydown", aoTeclar);
        return () => window.removeEventListener("keydown", aoTeclar);
    }, [aberto, aoFechar]);

    const aoReiniciar = () => {
        reiniciarJogo();
        aoFechar();
    };

    if (!aberto) return null;

    return (
        <div className="modal-fundo" onClick={aoFechar}>
            <div className="modal-caixa" role="dialog" aria-modal="true"
                 onClick={(e) => e.stopPropagation()}>
                <button className="modal-fechar" onClick={aoFechar} aria-label="Fechar" title="Fechar">⨉</button>
                <h1>Configurações</h1>
                <PainelConfiguracoes />
                <div>
                    <button className="botao-pixel" onClick={aoReiniciar}><img className="icone-botao" src={reiniciar}/>Reiniciar</button>
                    <Link className="botao-pixel" to="/"><img className="icone-botao" src={casa}/>Início</Link>
                </div>
            </div>
        </div>
    );
};

export default ModalConfiguracoes;