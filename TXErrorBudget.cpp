// ============================================================
// TX Engine — Technologic Experience Engine
// Técnica: Error Budget Management System (EBMS)
// ------------------------------------------------------------
// Objetivo:
// Gestionar el "error visual aceptable" por frame y por sistema
// para maximizar rendimiento sin degradación perceptible.
// ------------------------------------------------------------
// Filosofía:
// - El ojo humano tolera error si es coherente y estable
// - No todo error es igual (espacial, temporal, perceptual)
// - El motor decide cuánto error puede permitirse
// - Cada subsistema consume error como un presupuesto
// ============================================================

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>

namespace TX
{

// Tipos de error
enum class ErrorType : uint8_t
{
    Spatial,    // geometría, LOD, sombras
    Temporal,   // jitter, reproyección
    Shading,    // iluminación, BRDF
    Reflection, // reflejos, SSR
    Volumetric, // niebla, partículas
    Count
};

static constexpr uint32_t ERROR_TYPE_COUNT = static_cast<uint32_t>(ErrorType::Count);

// Presupuesto de error por tipo
struct ErrorBudget
{
    float Current;   // error actual acumulado
    float Limit;     // máximo aceptable
};

// Estado perceptual del frame
struct PerceptualState
{
    float CameraVelocity;   // movimiento de cámara
    float FocusDepth;       // profundidad dominante
    float Luminance;        // brillo medio
};

// Sistema principal
class ErrorBudgetSystem
{
public:
    ErrorBudgetSystem()
    {
        Reset();
    }

    void Reset()
    {
        for (uint32_t i = 0; i < ERROR_TYPE_COUNT; ++i)
        {
            Budgets[i].Current = 0.0f;
            Budgets[i].Limit   = BaseLimits[i];
        }
    }

    // Ajuste dinámico según percepción
    void AdaptToPerception(const PerceptualState& p)
    {
        // Más movimiento = más tolerancia temporal
        Budgets[(uint32_t)ErrorType::Temporal].Limit =
            BaseLimits[(uint32_t)ErrorType::Temporal] * (1.0f + p.CameraVelocity);

        // Más brillo = sombras menos críticas
        Budgets[(uint32_t)ErrorType::Spatial].Limit =
            BaseLimits[(uint32_t)ErrorType::Spatial] * (1.0f + p.Luminance * 0.5f);

        // Profundidad lejana = menos precisión en reflejos
        Budgets[(uint32_t)ErrorType::Reflection].Limit =
            BaseLimits[(uint32_t)ErrorType::Reflection] * (1.0f + p.FocusDepth);
    }

    // Solicitud de error por subsistema
    bool Request(ErrorType type, float amount)
    {
        ErrorBudget& B = Budgets[(uint32_t)type];

        if (B.Current + amount > B.Limit)
            return false;

        B.Current += amount;
        return true;
    }

    // Ratio de saturación (para AdaptiveQuality / PASS)
    float Saturation() const
    {
        float maxSat = 0.0f;

        for (uint32_t i = 0; i < ERROR_TYPE_COUNT; ++i)
        {
            maxSat = std::max(maxSat, Budgets[i].Current / Budgets[i].Limit);
        }

        return maxSat;
    }

    // Debug / Telemetría
    float GetUsage(ErrorType type) const
    {
        return Budgets[(uint32_t)type].Current / Budgets[(uint32_t)type].Limit;
    }

private:
    ErrorBudget Budgets[ERROR_TYPE_COUNT];

    // Límites base (tuneables por plataforma)
    static constexpr float BaseLimits[ERROR_TYPE_COUNT] =
    {
        1.0f, // Spatial
        0.8f, // Temporal
        0.6f, // Shading
        0.5f, // Reflection
        0.7f  // Volumetric
    };
};


// Ejemplo de uso
// if (ErrorSystem.Request(ErrorType::Spatial, lodError))
//     ApplyLOD();
// else
//     IncreaseLOD();
}
