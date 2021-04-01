#ifndef PALETTES_SPEC_H
#define PALETTES_SPEC_H

/// Non-standard palettes
/// Created By: Elijah Vlasov

#include "special/spec_BlGnYe.h"
#include "special/spec_BlYeRd.h"
#include "special/spec_GnYe.h"


const IPalette* const ppalettes_spec[] = { &palette_spec_BlYeRd, &palette_spec_BlGnYe, &palette_spec_GnYe };
const IPalette* const ppalettes_spec_inv[] = { &palette_spec_BlYeRd_inv, &palette_spec_BlGnYe_inv, &palette_spec_GnYe_inv };
const char*     const ppalettenames_spec[] = { "spec_BlYeRd", "spec_BlGnYe", "spec_GnYe" };

#endif // PALETTES_SPEC_H
