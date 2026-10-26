import { useContext } from "react";
import { ConfiguracoesContext } from "../contexts/ConfiguracoesContext";

export const useConfiguracoes = () => {
    const contexto = useContext(ConfiguracoesContext);
    if (!contexto) throw new Error("useConfiguracoes precisa estar dentro do ConfiguracoesProvider");
    return contexto;
};