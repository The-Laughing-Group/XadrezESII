import { useConfiguracoes } from "../hooks/useConfiguracoes";
import { useSom } from "../hooks/useSom";

const PainelConfiguracoes = () => {
    const { config, atualizar } = useConfiguracoes();
    const tocarMovimento = useSom("movimento");

    return (
        <>
            <div className="sections-config">
                <section className="grupo-config">
                    <h2>Visual</h2>

                    <label className="linha-config">
                        <span>Tamanho do tabuleiro</span>
                        <input type="range" min="0.6" max="1" step="0.05"
                            value={config.escala}
                            onChange={(e) => atualizar({ escala: Number(e.target.value) })} />
                    </label>

                    <label className="linha-config">
                        <span>Histórico de lances</span>
                        <input type="checkbox" className="interruptor"
                            checked={config.mostrarHistorico}
                            onChange={(e) => atualizar({ mostrarHistorico: e.target.checked })} />
                    </label>
                </section>

                <section className="grupo-config">
                    <h2>Sons</h2>

                    <label className="linha-config">
                        <span>Efeitos sonoros</span>
                        <input type="checkbox" className="interruptor"
                            checked={config.somLigado}
                            onChange={(e) => atualizar({ somLigado: e.target.checked })} />
                    </label>

                    <label className="linha-config">
                        <span>
                            Volume dos efeitos <span aria-hidden="true">{Math.round(config.volume * 100)}%</span>
                        </span>
                        <input type="range" min="0" max="1" step="0.05"
                            value={config.volume}
                            aria-valuetext={`${Math.round(config.volume * 100)}%`}
                            disabled={!config.somLigado}
                            onChange={(e) => atualizar({ volume: Number(e.target.value) })}
                            onPointerUp={tocarMovimento} />
                    </label>

                    <label className="linha-config">
                        <span>Música ambiente</span>
                        <input type="checkbox" className="interruptor"
                            checked={config.musicaLigada}
                            onChange={(e) => atualizar({ musicaLigada: e.target.checked })} />
                    </label>

                    <label className="linha-config">
                        <span>
                            Volume da música <span aria-hidden="true">{Math.round(config.volumeMusica * 100)}%</span>
                        </span>
                        <input type="range" min="0" max="1" step="0.05"
                            value={config.volumeMusica}
                            aria-valuetext={`${Math.round(config.volumeMusica * 100)}%`}
                            disabled={!config.musicaLigada}
                            onChange={(e) => atualizar({ volumeMusica: Number(e.target.value) })} />
                    </label>
                </section>
            </div>
        </>
    );
};

export default PainelConfiguracoes;