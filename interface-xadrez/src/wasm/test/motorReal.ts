// Ferramentas compartilhadas pelos testes de INTEGRAÇÃO front <-> motor de regras.
//
// A ideia: aqui o motor C++ (chess_engine.wasm) é o de VERDADE. Nada de mock do motor.
// Só trocamos o que o jsdom não tem: o carregamento do .wasm (via fetch), Audio e Worker.

import { readFileSync } from 'node:fs'
import { dirname, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'
import { vi } from 'vitest'
import criarModulo from '../chess_engine.js'
import type { JogoWasm, ModuloXadrez } from '../chess_engine.js'

// ---------------------------------------------------------------------------
// 1. Carregar o WebAssembly de verdade dentro do Vitest
// ---------------------------------------------------------------------------
// O chess_engine.js foi gerado para a WEB: ele baixa o .wasm com fetch(). No Node/jsdom
// esse fetch não sabe abrir arquivo local, então respondemos nós mesmos com os bytes do disco.
// Casamos só pelo NOME do arquivo, assim funciona qualquer que seja a URL que o Vite montar.

const CAMINHO_WASM = resolve(dirname(fileURLToPath(import.meta.url)), '..', 'chess_engine.wasm')
const ER_WASM = /chess_engine\.wasm(\?.*)?$/

let bytesDoWasm: Uint8Array<ArrayBuffer> | null = null
let fetchInstalado = false

function urlDe(entrada: RequestInfo | URL): string {
    if (typeof entrada === 'string') return entrada
    if ('href' in entrada) return entrada.href
    return entrada.url
}

export function instalarFetchDoWasm(): void {
    if (fetchInstalado) return
    fetchInstalado = true

    const fetchOriginal = globalThis.fetch
    globalThis.fetch = (async (entrada: RequestInfo | URL, init?: RequestInit) => {
        if (ER_WASM.test(urlDe(entrada))) {
            bytesDoWasm ??= new Uint8Array(readFileSync(CAMINHO_WASM))
            // application/wasm é exigido pelo WebAssembly.instantiateStreaming
            return new Response(bytesDoWasm, { headers: { 'Content-Type': 'application/wasm' } })
        }
        return fetchOriginal(entrada, init)
    }) as typeof fetch
}

let modulo: Promise<ModuloXadrez> | null = null

/** O módulo WASM real (carregado uma única vez por arquivo de teste). */
export function carregarMotorReal(): Promise<ModuloXadrez> {
    instalarFetchDoWasm()
    modulo ??= criarModulo()
    return modulo
}

/** Um Jogo "cru" do motor, opcionalmente já numa posição. Serve de ORÁCULO nos testes. */
export async function novoJogo(fen?: string): Promise<JogoWasm> {
    const jogo = new (await carregarMotorReal()).Jogo()
    if (fen && !jogo.carregarFen(fen)) throw new Error(`FEN recusado pelo motor: ${fen}`)
    return jogo
}

// ---------------------------------------------------------------------------
// 2. Posições usadas nos testes (todas conferidas contra o motor real)
// ---------------------------------------------------------------------------

export const FEN_INICIAL = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

export const FENS = {
    roques: 'r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1',
    enPassant: '4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1',
    promocao: '8/P6k/8/8/8/8/8/4K3 w - - 0 1',                 // a7-a8 promove
    capturaPromocao: '1n2k3/P7/8/8/8/8/8/4K3 w - - 0 1',       // a7xb8 promove
    cravada: '4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1',              // torre e2 cravada pela torre e8
    doisCavalos: '4k3/8/8/8/8/5N2/8/1N2K3 w - - 0 1',          // b1 e f3 chegam em d2
    quaseAfogamento: '7k/Q7/6K1/8/8/8/8/8 w - - 0 1',          // Df7 afoga
    jaAfogada: '7k/5Q2/6K1/8/8/8/8/8 b - - 0 1',
    quaseMaterialInsuficiente: '8/8/8/6k1/8/8/4p3/4K3 w - - 0 1', // Rxe2 sobra rei x rei
    quaseRegra50: '4k3/8/8/8/8/8/R7/4K3 w - - 99 80',          // Ta3 completa 100 meios-lances
} as const

// Estados que o front sabe desenhar (espelha EstadoDoJogo do chess_engine.d.ts).
export const ESTADOS_CONHECIDOS_PELO_FRONT = [
    'em_andamento', 'xeque_mate', 'afogamento', 'material_insuficiente', 'regra_dos_50', 'repeticao',
] as const

/** Todos os lances legais da posição, pela API pública do motor (ordenados, p/ ficar determinístico). */
export function todosOsLances(jogo: JogoWasm): string[] {
    const lances: string[] = []
    for (const coluna of 'abcdefgh') {
        for (const linha of '12345678') lances.push(...jogo.lancesDe(coluna + linha))
    }
    return lances.sort()
}

/** Toca uma lista de lances no motor; falha na hora se algum for recusado. */
export function jogarTodos(jogo: JogoWasm, lances: string[]): void {
    for (const uci of lances) {
        if (!jogo.tentarLance(uci)) throw new Error(`motor recusou ${uci} em ${jogo.fen()}`)
    }
}

// ---------------------------------------------------------------------------
// 3. Dublês do navegador que o jsdom não implementa
// ---------------------------------------------------------------------------

/** Audio falso que anota quem foi criado e se tocou (useSom e useMusica usam Audio). */
export class AudioFalso {
    static instancias: AudioFalso[] = []
    static limpar() { AudioFalso.instancias = [] }

    /** Quantas vezes o som com esse arquivo foi realmente tocado. */
    static vezesQueTocou(src: string): number {
        return AudioFalso.instancias.filter(a => a.src === src && a.play.mock.calls.length > 0).length
    }

    src: string
    volume = 1
    loop = false
    play = vi.fn(() => Promise.resolve())
    pause = vi.fn()

    constructor(src: string) {
        this.src = src
        AudioFalso.instancias.push(this)
    }
}

/** O usePartida cria um Worker (usado só pela IA). Aqui ele existe, mas não faz nada. */
export class WorkerFalso {
    static instancias: WorkerFalso[] = []
    onmessage: ((e: MessageEvent) => void) | null = null
    postMessage = vi.fn()
    terminate = vi.fn()
    constructor() {
        WorkerFalso.instancias.push(this)
    }
}

export function instalarDubles(): void {
    AudioFalso.limpar()
    WorkerFalso.instancias = []
    vi.stubGlobal('Audio', AudioFalso)
    vi.stubGlobal('Worker', WorkerFalso)
}

// ---------------------------------------------------------------------------
// 4. Ler o que o usuário VÊ na tela (DOM do Tabuleiro / histórico / indicador)
// ---------------------------------------------------------------------------

// Conversão feita aqui de propósito (sem importar utils/notacao): o teste não pode
// depender do código que ele está testando para calcular a resposta certa.
const rowCol = (uci: string) => ({ col: uci.charCodeAt(0) - 97, row: 8 - Number(uci[1]) })
const uciDe = (row: number, col: number) => String.fromCharCode(97 + col) + (8 - row)

export function casaEl(uci: string): HTMLElement {
    const { row, col } = rowCol(uci)
    const el = document.querySelector<HTMLElement>(`.casa[data-row="${row}"][data-col="${col}"]`)
    if (!el) throw new Error(`casa ${uci} não está na tela`)
    return el
}

/** Letra da peça desenhada na casa ('P', 'n'...) ou '.' se vazia. */
export function pecaEm(uci: string): string {
    return casaEl(uci).querySelector('img.peca')?.getAttribute('data-tipo') ?? '.'
}

/** O tabuleiro como está DESENHADO: mesmo formato do tabuleiro() do motor. */
export function tabuleiroDoDom(): string[] {
    return Array.from({ length: 8 }, (_, row) =>
        Array.from({ length: 8 }, (_, col) => pecaEm(uciDe(row, col))).join(''),
    )
}

/** Casas destacadas como "destino possível", ordenadas. */
export function destinosMarcados(): string[] {
    return Array.from(document.querySelectorAll<HTMLElement>('.movimento-possivel-branco, .movimento-possivel-preto'))
        .map(el => uciDe(Number(el.dataset.row), Number(el.dataset.col)))
        .sort()
}

export function casaSelecionadaNaTela(): string | null {
    const el = document.querySelector<HTMLElement>('.casa-selecionada')
    return el ? uciDe(Number(el.dataset.row), Number(el.dataset.col)) : null
}

export function textoDoIndicador(): string {
    return document.querySelector('.indicador-vez')?.textContent?.replace(/\s+/g, ' ').trim() ?? ''
}

/** Colunas "Brancas" e "Pretas" do histórico de lances. */
export function historicoDaTela(): { brancas: string[]; pretas: string[] } {
    const colunas = document.querySelectorAll('#historico-movimentos .coluna-movimentos')
    const ler = (i: number) => Array.from(colunas[i].querySelectorAll('p')).map(p => p.textContent?.trim() ?? '')
    return { brancas: ler(1), pretas: ler(2) }
}

/** Gerador pseudoaleatório com semente fixa (mulberry32): mesma partida em toda execução. */
export function criarSorteio(semente: number): () => number {
    let s = semente >>> 0
    return () => {
        s = (s + 0x6d2b79f5) >>> 0
        let t = s
        t = Math.imul(t ^ (t >>> 15), t | 1)
        t ^= t + Math.imul(t ^ (t >>> 7), t | 61)
        return ((t ^ (t >>> 14)) >>> 0) / 4294967296
    }
}