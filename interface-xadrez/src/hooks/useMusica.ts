import { useEffect, useRef } from "react";
import { useConfiguracoes } from "./useConfiguracoes";     
import musica from "../assets/sons/musica.m4a";

export const useMusica = () => {
    const { config } = useConfiguracoes();
    const audioRef = useRef<HTMLAudioElement | null>(null);

    useEffect(() => {
        const audio = new Audio(musica);
        audio.loop = true;
        audioRef.current = audio;

        return () => {
            audio.pause();
            audioRef.current = null;
        };
    }, []);

    useEffect(() => {
        const audio = audioRef.current;
        if (!audio) return;

        audio.volume = config.volumeMusica;

        if (!config.musicaLigada) {
            audio.pause();
            return;
        }

        const tocar = () => audio.play().catch(() => {});

        audio.play().catch(() => window.addEventListener("pointerdown", tocar, { once: true }));

        return () => window.removeEventListener("pointerdown", tocar);
    }, [config.musicaLigada, config.volumeMusica]);
};