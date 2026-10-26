import { useConfiguracoes } from "../contexts/ConfiguracaoContext";
import { useSom } from "../hooks/useSom";

const PainelConfiguracoes = () => {
    const { config, atualizar } = useConfiguracoes();
    const tocarMovimento = useSom();

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
                    <h2>Som</h2>

                    <label className="linha-config">
                        <span>Efeitos sonoros</span>
                        <input type="checkbox" className="interruptor"
                            checked={config.somLigado}
                            onChange={(e) => atualizar({ somLigado: e.target.checked })} />
                    </label>

                    <label className="linha-config">
                        <span>Volume {Math.round(config.volume * 100)}%</span>
                        <input type="range" min="0" max="1" step="0.05"
                            value={config.volume}
                            disabled={!config.somLigado}
                            onChange={(e) => atualizar({ volume: Number(e.target.value) })}
                            onPointerUp={tocarMovimento} />
                    </label>
                </section>
            </div>
        </>
    );
};

export default PainelConfiguracoes;