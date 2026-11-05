import { createContext } from "react";

export type Configuracoes = {
    escala: number;             
    mostrarHistorico: boolean;
    somLigado: boolean;
    volume: number;             
};

export const PADRAO: Configuracoes = {
    escala: 1,
    mostrarHistorico: true,
    somLigado: true,
    volume: 0.5,
};

export type Contexto = {
    config: Configuracoes;
    atualizar: (mudanca: Partial<Configuracoes>) => void;
};

export const ConfiguracoesContext = createContext<Contexto | null>(null);