import { useEffect, useRef } from "react";
import { useConfiguracoes } from "./useConfiguracoes";     
import musica from "../assets/sons/musica.m4a";


const GESTOS = ["pointerdown", "pointerup", "keydown"] as const;

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
        
        let ativo = true;                                              

        const tocar = () => {
            if (ativo) audio.play().catch(() => {});                  
        };
        
        audio.play().catch(() => {
            if (!ativo) return;                                        
            GESTOS.forEach((g) => window.addEventListener(g, tocar, { once: true }));  
        });

        return () => {
            ativo = false;                                             
            GESTOS.forEach((g) => window.removeEventListener(g, tocar));               
        };
    }, [config.musicaLigada, config.volumeMusica]);
};