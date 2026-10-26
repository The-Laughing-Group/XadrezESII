import { useEffect, useState, type ReactNode } from "react";
import { ConfiguracoesContext, PADRAO, type Configuracoes } from "./ConfiguracoesContext";

export const ConfiguracoesProvider = ({ children }: { children: ReactNode }) => {

    const [config, setConfig] = useState<Configuracoes>(() => {
        try {
            return { ...PADRAO, ...JSON.parse(localStorage.getItem("configuracoes") ?? "{}") };
        } catch {
            return PADRAO;
        }
    });

    useEffect(() => {
        localStorage.setItem("configuracoes", JSON.stringify(config));

        const raiz = document.documentElement;
        raiz.style.setProperty("--escala", String(config.escala));
        raiz.classList.toggle("sem-historico", !config.mostrarHistorico);
    }, [config]);

    const atualizar = (mudanca: Partial<Configuracoes>) => {
        setConfig((atual) => ({ ...atual, ...mudanca }));
    };

    return (
        <ConfiguracoesContext.Provider value={{ config, atualizar }}>
            {children}
        </ConfiguracoesContext.Provider>
    );
};