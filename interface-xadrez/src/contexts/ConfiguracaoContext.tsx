import { createContext, useContext, useEffect, useState, type ReactNode } from "react";

type Configuracoes = {
    escala: number;             
    mostrarHistorico: boolean;
    somLigado: boolean;
    volume: number;             
};

const PADRAO: Configuracoes = {
    escala: 1,
    mostrarHistorico: true,
    somLigado: true,
    volume: 0.5,
};

type Contexto = {
    config: Configuracoes;
    atualizar: (mudanca: Partial<Configuracoes>) => void;
};

const ConfiguracoesContext = createContext<Contexto | null>(null);

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

export const useConfiguracoes = () => {
    const contexto = useContext(ConfiguracoesContext);
    if (!contexto) throw new Error("useConfiguracoes precisa estar dentro do ConfiguracoesProvider");
    return contexto;
};