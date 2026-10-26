import { useConfiguracoes } from "../contexts/ConfiguracaoContext";
import movimento from "../assets/sons/movimento.m4a";
import promocao from "../assets/sons/promocao.mp3";

export const useSom = (tipo: String) => {
    const { config } = useConfiguracoes();

    let audio: HTMLAudioElement;

    return () => {
        if (!config.somLigado) return;

        if(tipo === "movimento") {
            audio = new Audio(movimento);
        }
        else if(tipo === "promocao") {
            audio = new Audio(promocao);
        }

        audio.volume = config.volume;
        audio.play().catch(() => {});   
    };
};