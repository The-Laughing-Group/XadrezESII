interface HistoricoMovimentosProps {
    historicoMovimentos: string[];
}

const HistoricoMovimentos = ({ historicoMovimentos }: HistoricoMovimentosProps) => {
    return (
        <div id="historico-movimentos">
            <div className="coluna-movimentos">
                <h3>Rodadas</h3>
                {historicoMovimentos
                    .filter((_, index) => index % 2 === 0)
                    .map((_,index) => (
                        <p key={index + 1}>
                            {index + 1}. 
                        </p>
                    ))}
            </div>
            <div className="coluna-movimentos">
                <h3>Brancas</h3>

                {historicoMovimentos
                    .filter((_, index) => index % 2 === 0)
                    .map((movimento, index) => (
                        <p key={index}>
                            {movimento}
                        </p>
                    ))}
            </div>
            <div className="coluna-movimentos">
                <h3>Pretas</h3>
                {historicoMovimentos
                    .filter((_, index) => index % 2 !== 0)
                    .map((movimento, index) => (
                        <p key={index + 1}>
                            {movimento}
                        </p>
                    ))}
            </div>
        </div>
    )
}
export default HistoricoMovimentos