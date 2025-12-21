// ============================================================
// TX Engine — Technologic Experience Engine
// Objetivo:
// Establecer un presupuesto estricto de memoria por frame,
// evitando picos, fragmentación y comportamiento no determinista.
// Cada frame sabe exactamente cuánta memoria puede consumir.
// ------------------------------------------------------------
// Filosofía:
// - El frame es la unidad sagrada del motor
// - Ningún subsistema puede exceder su cuota
// - La memoria se gasta como tiempo: con presupuesto
// - Todo es predecible, medible y reversible
// ============================================================

#include <cstdint>
#include <cstring>

namespace TX
{

// ------------------------------------------------------------
// Constantes
// ------------------------------------------------------------

constexpr uint64_t KB = 1024;
constexpr uint64_t MB = 1024 * KB;

// ------------------------------------------------------------
// Categorías de gasto por frame
// ------------------------------------------------------------

enum class FrameMemoryDomain : uint8_t
{
    Geometry,
    Animation,
    Textures,
    Particles,
    AI,
    Audio,
    Physics,
    UI,
    Count
};

// ------------------------------------------------------------
// Presupuesto por dominio
// ------------------------------------------------------------

struct FrameMemoryBudget
{
    uint64_t MaxBytes;
    uint64_t UsedBytes;
};

// ------------------------------------------------------------
// Sistema principal
// ------------------------------------------------------------

class FrameMemoryBudgetSystem
{
public:
    FrameMemoryBudgetSystem()
    {
        Reset();
    }

    // Inicialización con presupuesto total del frame
    void Initialize(uint64_t totalFrameBudget)
    {
        TotalBudget = totalFrameBudget;

        // Distribución base (ajustable por plataforma)
        Budgets[(uint8_t)FrameMemoryDomain::Geometry]   = { totalFrameBudget * 30 / 100, 0 };
        Budgets[(uint8_t)FrameMemoryDomain::Textures]   = { totalFrameBudget * 25 / 100, 0 };
        Budgets[(uint8_t)FrameMemoryDomain::Animation]  = { totalFrameBudget * 10 / 100, 0 };
        Budgets[(uint8_t)FrameMemoryDomain::Particles]  = { totalFrameBudget * 8  / 100, 0 };
        Budgets[(uint8_t)FrameMemoryDomain::Physics]    = { totalFrameBudget * 8  / 100, 0 };
        Budgets[(uint8_t)FrameMemoryDomain::AI]         = { totalFrameBudget * 7  / 100, 0 };
        Budgets[(uint8_t)FrameMemoryDomain::Audio]      = { totalFrameBudget * 6  / 100, 0 };
        Budgets[(uint8_t)FrameMemoryDomain::UI]         = { totalFrameBudget * 6  / 100, 0 };
    }

    // Reinicio por frame
    void BeginFrame()
    {
        for (uint8_t i = 0; i < (uint8_t)FrameMemoryDomain::Count; ++i)
            Budgets[i].UsedBytes = 0;
    }

    // Solicitud de memoria
    bool Request(FrameMemoryDomain domain, uint64_t bytes)
    {
        FrameMemoryBudget& budget = Budgets[(uint8_t)domain];

        if (budget.UsedBytes + bytes > budget.MaxBytes)
            return false;

        budget.UsedBytes += bytes;
        return true;
    }

    // Consulta de estado
    uint64_t GetRemaining(FrameMemoryDomain domain) const
    {
        const FrameMemoryBudget& budget = Budgets[(uint8_t)domain];
        return (budget.MaxBytes > budget.UsedBytes)
             ? (budget.MaxBytes - budget.UsedBytes)
             : 0;
    }

    float GetUsageRatio(FrameMemoryDomain domain) const
    {
        const FrameMemoryBudget& budget = Budgets[(uint8_t)domain];
        return (float)budget.UsedBytes / (float)budget.MaxBytes;
    }

    // Evaluación de riesgo
    bool IsDomainCritical(FrameMemoryDomain domain) const
    {
        return GetUsageRatio(domain) > 0.9f;
    }

    // Presupuesto total restante
    uint64_t GetTotalRemaining() const
    {
        uint64_t used = 0;
        for (uint8_t i = 0; i < (uint8_t)FrameMemoryDomain::Count; ++i)
            used += Budgets[i].UsedBytes;

        return (TotalBudget > used) ? (TotalBudget - used) : 0;
    }

    void Reset()
    {
        std::memset(Budgets, 0, sizeof(Budgets));
        TotalBudget = 0;
    }

private:
    FrameMemoryBudget Budgets[(uint8_t)FrameMemoryDomain::Count];
    uint64_t TotalBudget;
};
}