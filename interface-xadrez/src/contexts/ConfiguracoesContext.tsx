import { createContext } from "react";

export type Configuracoes = {
    escala: number;             
    mostrarHistorico: boolean;
    somLigado: boolean;
    volume: number;    
    musicaLigada: boolean;  
    volumeMusica: number;      
};

export const PADRAO: Configuracoes = {
    escala: 1,
    mostrarHistorico: true,
    somLigado: true,
    volume: 0.5,
    musicaLigada: true,
    volumeMusica: 0.3, 
};

export type Contexto = {
    config: Configuracoes;
    atualizar: (mudanca: Partial<Configuracoes>) => void;
};

export const ConfiguracoesContext = createContext<Contexto | null>(null);