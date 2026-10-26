import { useConfiguracoes } from "../hooks/useConfiguracoes";
import movimento from "../assets/sons/movimento.m4a";
import promocao from "../assets/sons/promocao.mp3";

const SONS = { movimento, promocao } as const;

type TipoSom = keyof typeof SONS;

export const useSom = (tipo: TipoSom = "movimento") => {
    const { config } = useConfiguracoes();

    return () => {
        if (!config.somLigado) return;

        const audio = new Audio(SONS[tipo]);

        audio.volume = config.volume;
        audio.play().catch(() => {});   
    };
};