import { useConfiguracoes } from "../contexts/ConfiguracaoContext";
import movimento from "../assets/sons/movimento.mp3";

export const useSom = () => {
    const { config } = useConfiguracoes();

    return () => {
        if (!config.somLigado) return;

        const audio = new Audio(movimento);
        audio.volume = config.volume;
        audio.play().catch(() => {});   
    };
};