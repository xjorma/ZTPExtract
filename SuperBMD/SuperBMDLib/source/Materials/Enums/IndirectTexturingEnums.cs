﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperBMDLib.Materials.Enums
{
    public enum IndirectFormat
    {
        ITF_8, // 8 bit
        ITF_5, // 5 bit
        ITF_4, // 4 bit
        ITF_3 // 3 bit
    }

    public enum IndirectBias
    {
        None,
        ITB_S,
        ITB_T,
        ITB_ST,
        ITB_U,
        ITB_SU,
        ITB_TU,
        ITB_STU
    }

    public enum IndirectAlpha
    {
        ITBA_OFF,

        ITBA_S,
        ITBA_T,
        ITBA_U
    }

    public enum IndirectMatrix
    {
        ITM_OFF,

        ITM_0,
        ITM_1,
        ITM_2,

        ITM_S0 = 5,
        ITM_S1,
        ITM_S2,

        ITM_T0 = 9,
        ITM_T1,
        ITM_T2
    }

    public enum IndirectWrap
    {
        ITW_OFF,

        ITW_256,
        ITW_128,
        ITW_64,
        ITW_32,
        ITW_16,
        ITW_0
    }

    public enum IndirectScale
    {
        ITS_1,      // 1
        ITS_2,      // 1/2
        ITS_4,      // 1/4
        ITS_8,      // 1/8
        ITS_16,     // 1/16
        ITS_32,     // 1/32
        ITS_64,     // 1/64
        ITS_128,    // 1/128
        ITS_256     // 1/256
    }
}
