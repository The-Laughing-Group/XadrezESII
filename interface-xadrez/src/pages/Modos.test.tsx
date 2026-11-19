import { describe, it, expect, vi, beforeEach } from "vitest";
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { MemoryRouter } from "react-router-dom";

import Modos from "./Modos";

// Troca só o useNavigate por um espião; o resto do react-router (Link, etc.) continua real.
const navigateMock = vi.fn();
vi.mock("react-router-dom", async (importOriginal) => {
  const original = await importOriginal<typeof import("react-router-dom")>();
  return { ...original, useNavigate: () => navigateMock };
});

const renderizar = () =>
  render(
    <MemoryRouter>
      <Modos />
    </MemoryRouter>
  );

// atalhos para achar os elementos pelo nome que o jogador enxerga
const botaoModo = (name: RegExp) => screen.getByRole("checkbox", { name: name });
const dificuldade = (name: string) => screen.getByRole("checkbox", { name: name });
const botaoJogar = () => screen.getByRole("button", { name: "Jogar" });

describe("Modos", () => {
  beforeEach(() => {
    navigateMock.mockClear();
  });

  describe("estado inicial", () => {
    it("começa em 2 Players", () => {
      renderizar();
      expect(botaoModo(/2 players/i)).toHaveAttribute("aria-checked", "true");
      expect(botaoModo(/player x ia/i)).toHaveAttribute("aria-checked", "false");
      expect(botaoModo(/2 players/i)).toHaveClass("selecionado");
    });

    it("começa com as três dificuldades desabilitadas e desmarcadas", () => {
      renderizar();
      for (const nome of ["Fácil", "Médio", "Difícil"]) {
        expect(dificuldade(nome)).toBeDisabled();
        expect(dificuldade(nome)).not.toBeChecked();
      }
    });
  });

  describe("botões de modo", () => {
    it("selecionar Player x IA desmarca 2 Players", async () => {
      const user = userEvent.setup();
      renderizar();

      await user.click(botaoModo(/player x ia/i));

      expect(botaoModo(/player x ia/i)).toHaveAttribute("aria-checked", "true");
      expect(botaoModo(/player x ia/i)).toHaveClass("selecionado");
      expect(botaoModo(/2 players/i)).toHaveAttribute("aria-checked", "false");
      expect(botaoModo(/2 players/i)).not.toHaveClass("selecionado");
    });

    it("voltar para 2 Players desmarca Player x IA", async () => {
      const user = userEvent.setup();
      renderizar();

      await user.click(botaoModo(/player x ia/i));
      await user.click(botaoModo(/2 players/i));

      expect(botaoModo(/2 players/i)).toHaveAttribute("aria-checked", "true");
      expect(botaoModo(/player x ia/i)).toHaveAttribute("aria-checked", "false");
    });

    it("clicar duas vezes no modo já selecionado não o desmarca", async () => {
      const user = userEvent.setup();
      renderizar();

      await user.click(botaoModo(/2 players/i));
      await user.click(botaoModo(/2 players/i));

      expect(botaoModo(/2 players/i)).toHaveAttribute("aria-checked", "true");
    });
  });

  describe("dificuldade", () => {
    it("habilita as dificuldades ao escolher IA, com Médio como padrão", async () => {
      const user = userEvent.setup();
      renderizar();

      await user.click(botaoModo(/player x ia/i));

      for (const nome of ["Fácil", "Médio", "Difícil"]) {
        expect(dificuldade(nome)).toBeEnabled();
      }
      expect(dificuldade("Médio")).toBeChecked();
      expect(dificuldade("Fácil")).not.toBeChecked();
      expect(dificuldade("Difícil")).not.toBeChecked();
    });

    it("só uma dificuldade fica marcada por vez", async () => {
      const user = userEvent.setup();
      renderizar();
      await user.click(botaoModo(/player x ia/i));

      await user.click(dificuldade("Difícil"));
      expect(dificuldade("Difícil")).toBeChecked();
      expect(dificuldade("Médio")).not.toBeChecked();
      expect(dificuldade("Fácil")).not.toBeChecked();

      await user.click(dificuldade("Fácil"));
      expect(dificuldade("Fácil")).toBeChecked();
      expect(dificuldade("Difícil")).not.toBeChecked();
    });

    it("clicar na dificuldade já marcada não a desmarca", async () => {
      const user = userEvent.setup();
      renderizar();
      await user.click(botaoModo(/player x ia/i));

      await user.click(dificuldade("Médio"));

      expect(dificuldade("Médio")).toBeChecked();
    });

    it("voltar para 2 Players desabilita e desmarca as dificuldades", async () => {
      const user = userEvent.setup();
      renderizar();
      await user.click(botaoModo(/player x ia/i));
      await user.click(dificuldade("Difícil"));

      await user.click(botaoModo(/2 players/i));

      for (const nome of ["Fácil", "Médio", "Difícil"]) {
        expect(dificuldade(nome)).toBeDisabled();
        expect(dificuldade(nome)).not.toBeChecked();
      }
    });

    it("lembra a dificuldade escolhida ao voltar para IA", async () => {
      const user = userEvent.setup();
      renderizar();
      await user.click(botaoModo(/player x ia/i));
      await user.click(dificuldade("Fácil"));
      await user.click(botaoModo(/2 players/i));

      await user.click(botaoModo(/player x ia/i));

      expect(dificuldade("Fácil")).toBeChecked();
    });

    it("clicar numa dificuldade desabilitada não muda nada", async () => {
      const user = userEvent.setup();
      renderizar();

      await user.click(dificuldade("Difícil")); // modo 2 Players: está desabilitada

      expect(dificuldade("Difícil")).not.toBeChecked();
    });
  });

  describe("botão Jogar", () => {
    it("em 2 Players navega sem o parâmetro de dificuldade", async () => {
      const user = userEvent.setup();
      renderizar();

      await user.click(botaoJogar());

      expect(navigateMock).toHaveBeenCalledTimes(1);
      expect(navigateMock).toHaveBeenCalledWith("/game?modo=twoplayers");
    });

    it("em IA, com a dificuldade padrão, navega com dificuldade=medio", async () => {
      const user = userEvent.setup();
      renderizar();
      await user.click(botaoModo(/player x ia/i));

      await user.click(botaoJogar());

      expect(navigateMock).toHaveBeenCalledWith("/game?modo=ia&dificuldade=medio");
    });

    it.each([
      ["Fácil", "facil"],
      ["Médio", "medio"],
      ["Difícil", "dificil"],
    ])("em IA com %s navega com dificuldade=%s", async (rotulo, valor) => {
      const user = userEvent.setup();
      renderizar();
      await user.click(botaoModo(/player x ia/i));
      await user.click(dificuldade(rotulo));

      await user.click(botaoJogar());

      expect(navigateMock).toHaveBeenCalledWith(`/game?modo=ia&dificuldade=${valor}`);
    });

    it("depois de escolher dificuldade e voltar para 2 Players, não envia dificuldade", async () => {
      const user = userEvent.setup();
      renderizar();
      await user.click(botaoModo(/player x ia/i));
      await user.click(dificuldade("Difícil"));
      await user.click(botaoModo(/2 players/i));

      await user.click(botaoJogar());

      expect(navigateMock).toHaveBeenCalledWith("/game?modo=twoplayers");
    });

    it("não navega antes de o jogador clicar em Jogar", async () => {
      const user = userEvent.setup();
      renderizar();

      await user.click(botaoModo(/player x ia/i));
      await user.click(dificuldade("Fácil"));

      expect(navigateMock).not.toHaveBeenCalled();
    });
  });

  describe("botão Voltar", () => {
    it("é um link acessível que leva para a tela inicial", () => {
      renderizar();
      expect(screen.getByRole("link", { name: "Voltar" })).toHaveAttribute("href", "/");
    });
  });
});
