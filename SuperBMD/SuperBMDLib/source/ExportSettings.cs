﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SuperBMDLib
{
    public class ExportSettings
    {
        public bool UseSkeletonRoot;

        public ExportSettings(bool export_skeleton_root)
        {
            UseSkeletonRoot = export_skeleton_root;
        }
    }
}
