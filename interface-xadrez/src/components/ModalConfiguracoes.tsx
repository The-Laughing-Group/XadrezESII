// ModalConfiguracoes.tsx
import { useEffect } from "react";
import PainelConfiguracoes from "./PainelConfiguracoes";

type Props = { aberto: boolean; aoFechar: () => void };

const ModalConfiguracoes = ({ aberto, aoFechar }: Props) => {
    useEffect(() => {
        if (!aberto) return;
        const aoTeclar = (e: KeyboardEvent) => e.key === "Escape" && aoFechar();
        window.addEventListener("keydown", aoTeclar);
        return () => window.removeEventListener("keydown", aoTeclar);
    }, [aberto, aoFechar]);

    if (!aberto) return null;

    return (
        <div className="modal-fundo" onClick={aoFechar}>
            <div className="modal-caixa" role="dialog" aria-modal="true"
                 onClick={(e) => e.stopPropagation()}>
                <button className="modal-fechar" onClick={aoFechar}>⨉</button>
                <h1>Configurações</h1>
                <PainelConfiguracoes />
            </div>
        </div>
    );
};

export default ModalConfiguracoes;