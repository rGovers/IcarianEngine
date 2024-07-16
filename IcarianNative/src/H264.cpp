#include "Rendering/Video/H264.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <glm/gtc/integer.hpp>

#include "Core/Bitfield.h"
#include "IcarianError.h"

namespace H264
{
    NALHeader ReadNALHeader(BitStream* a_bitstream)
    {
        IVERIFY(a_bitstream != nullptr);

        const uint32_t fBit = a_bitstream->u1();
        IVERIFY(fBit == 0);

        const NALHeader header =
        {
            .IDC = (e_NALRefIDC)a_bitstream->u(2),
            .Type = (e_NALUnitType)a_bitstream->u(5)
        };

        return header;
    }

    static void ReadScalingList(H264::BitStream* a_bitstream, int8_t* a_scalingList, uint32_t a_size, uint8_t* a_useDefaultScalingMatrixFlag, uint8_t a_index)
    {
        IVERIFY(a_bitstream != nullptr);
        IVERIFY(a_scalingList != nullptr);
        IVERIFY(a_useDefaultScalingMatrixFlag != nullptr);

        int32_t lastScale = 8;
    	int32_t nextScale = 8;
    	for (uint32_t i = 0; i < a_size; ++i)
    	{
    		if (nextScale != 0)
    		{
    			const int32_t deltaScale = a_bitstream->se();
    			nextScale = (lastScale + deltaScale + 256) % 256;
    			ITOGGLEBIT(i == 0 && nextScale == 0, *a_useDefaultScalingMatrixFlag, a_index);
    		}

            int32_t val;
            if (nextScale == 0)
            {
                val = lastScale;
            }
            else
            {
                val = nextScale;
            }

            a_scalingList[i] = val;
    		lastScale = val;
    	}
    }

    VUIHRD ReadHRDParameters(BitStream* a_bitstream)
	{
        VUIHRD hrd = { 0 };

		hrd.CPBCNTMinus1 = a_bitstream->ue();
		hrd.BitRateScale = a_bitstream->u<uint8_t>(4);
		hrd.CPBSizeScale = a_bitstream->u<uint8_t>(4);

		for (uint32_t i = 0; i <= hrd.CPBCNTMinus1; ++i)
		{
			hrd.BitRateValueMinus1[i] = a_bitstream->ue();
			hrd.CPBSizeValueMinus1[i] = a_bitstream->ue();
            ITOGGLEBIT(a_bitstream->u1(), hrd.CBRFlag, i);
		}
		hrd.InitialCPBRemovalDelayLengthMinus1 = a_bitstream->u<uint8_t>(5);
		hrd.CPBRemovalDelayLengthMinus1 = a_bitstream->u<uint8_t>(5);
		hrd.DPBOutputDelayLengthMinus1 = a_bitstream->u<uint8_t>(5);
		hrd.TimeOffsetLength = a_bitstream->u<uint8_t>(5);

        return hrd;
	}

    static SPSVUI ReadVUI(BitStream* a_bitstream)
    {
        IVERIFY(a_bitstream != nullptr);

        SPSVUI vui = { 0 };

        const uint8_t aspectRatio = a_bitstream->u1();
        if (aspectRatio)
        {
            vui.Flags |= SPSVUIFlags_AspectRatioInfoPresent;

            vui.AspectRatioIDC = a_bitstream->u<uint8_t>(8);
            if (vui.AspectRatioIDC == 255)
            {
                vui.SARWidth = a_bitstream->u<uint16_t>(16);
                vui.SARHeight = a_bitstream->u<uint16_t>(16);
            }
        }   

        const uint8_t overscanInfoPresent = a_bitstream->u1();
        if (overscanInfoPresent)
        {
            vui.Flags |= SPSVUIFlags_OverscanInfoPresent;

            const uint8_t overscanAppropriate = a_bitstream->u1();
            if (overscanAppropriate)
            {
                vui.Flags |= SPSVUIFlags_OverscanAppropriate;
            }
        }

        const uint8_t videoSignalType = a_bitstream->u1();
        if (videoSignalType)
        {
            vui.Flags |= SPSVUIFlags_VideoSignalTypePresent;

            vui.VideoFormat = a_bitstream->u<uint8_t>(3);

            const uint8_t fullRange = a_bitstream->u1();
            if (fullRange)
            {
                vui.Flags |= SPSVUIFlags_VideoFullRange;
            }

            const uint8_t colourDescription = a_bitstream->u1();
            if (colourDescription)
            {
                vui.Flags |= SPSVUIFlags_ColourDescriptionPresent;

                vui.ColourPrimaries = a_bitstream->u<uint8_t>(8);
                vui.TransferCharacteristics = a_bitstream->u<uint8_t>(8);
                vui.MatrixCoefficients = a_bitstream->u<uint8_t>(8);
            }
        }

        const uint8_t chromaLOC = a_bitstream->u1();
        if (chromaLOC)
        {
            vui.Flags |= H264::SPSVUIFlags_ChromaLOCInfoPresent;

            vui.ChromaSampleLOCTypeTopField = a_bitstream->ue();
            vui.ChromaSampleLOCTypeBottomField = a_bitstream->ue();
        }

        const uint8_t timingInfo = a_bitstream->u1();
        if (timingInfo)
        {
            vui.Flags |= SPSVUIFlags_TimingInfoPresent;

            vui.NumUnitsInTick = a_bitstream->u(32);
            vui.TimeScale = a_bitstream->u(32);

            const uint8_t fixedFrame = a_bitstream->u1();
            if (fixedFrame)
            {
                vui.Flags |= SPSVUIFlags_FixedFrameRate;
            }
        }

        const uint8_t nalHRD = a_bitstream->u1();
        if (nalHRD)
        {
            vui.Flags |= SPSVUIFlags_NALHRDParametersPresent;

            vui.NALHRD = ReadHRDParameters(a_bitstream);
        }

        const uint8_t vclHRD = a_bitstream->u1();
        if (vclHRD)
        {
            vui.Flags |= SPSVUIFlags_VCLHRDParametersPresent;

            vui.VCLHRD = ReadHRDParameters(a_bitstream);
        }

        if (nalHRD || vclHRD)
        {
            const uint8_t lowDelay = a_bitstream->u1();
            if (lowDelay)
            {
                vui.Flags |= SPSVUIFlags_LowDelayHRD;
            }
        }

        const uint8_t picStruct = a_bitstream->u1();
        if (picStruct)
        {
            vui.Flags |= SPSVUIFlags_PICStructPresent;
        }

        const uint8_t bitStream = a_bitstream->u1();
        if (bitStream)
        {
            vui.Flags |= SPSVUIFlags_BitstreamRestriction;

            const uint8_t motionVector = a_bitstream->u1();
            if (motionVector)
            {
                vui.Flags |= SPSVUIFlags_MotionVectorsOverPICBoundaries;
            }

            vui.MaxBytesPerPICDenom = a_bitstream->ue();
            vui.MaxBitsPerMBDenom = a_bitstream->ue();
            vui.Log2MaxMvLengthHorizontal = a_bitstream->ue();
            vui.Log2MaxMvLengthVertical = a_bitstream->ue();
            vui.NumReorderFrames = (uint8_t)a_bitstream->ue();
            vui.MaxDecFrameBuffering = (uint8_t)a_bitstream->ue();
        }

        return vui;
    }

    SPS ReadSPS(BitStream* a_bitstream)
    {
        IVERIFY(a_bitstream != nullptr);

        SPS sps = { 0 };

        sps.ProfileIDC = a_bitstream->u<uint8_t>(8);
        sps.ConstraintSetFlag = a_bitstream->u<uint8_t>(6);
        // Reserved
        a_bitstream->Ignore(2);
        sps.LevelIDC = a_bitstream->u<uint8_t>(8);
        sps.SeqParameterSetID = (uint8_t)a_bitstream->ue();

        if (sps.ProfileIDC == 100 || sps.ProfileIDC == 110 || sps.ProfileIDC == 122 || sps.ProfileIDC == 144)
        {
            sps.ChromaFormatIDC = (uint8_t)a_bitstream->ue();
            if (sps.ChromaFormatIDC == 3)
            {
                const uint8_t seperateColorPlane = a_bitstream->u1();
                if (seperateColorPlane)
                {
                    sps.Flags |= SPSFlags_SeparateColourPlane;
                }
            }

            sps.BitDepthLumaMinus8 = (uint8_t)a_bitstream->ue();
            sps.BitDepthChromaMinus8 = (uint8_t)a_bitstream->ue();
            const uint8_t qpPrime = a_bitstream->u1();
            if (qpPrime)
            {
                sps.Flags |= SPSFlags_QPPrimeYZeroTransformBypass;
            }
            const uint8_t seqScaleMatrix = a_bitstream->u1();
            if (seqScaleMatrix)
            {
                sps.Flags |= SPSFlags_SeqScalingMatrixPresent;   

                for (uint32_t i = 0; i < 8; ++i)
                {
                    const uint8_t flagSet = a_bitstream->u1();
                    if (flagSet)
                    {
                        ISETBIT(sps.SeqScalingListPresentFlag, i);
                        if (i < 6)
                        {
                            ReadScalingList(a_bitstream, sps.ScalingList4x4[i], 16, &sps.UseDefaultScalingMatrixFlag, i);
                        }
                        else
                        {
                            ReadScalingList(a_bitstream, sps.ScalingList8x8[i - 6], 64, &sps.UseDefaultScalingMatrixFlag, i);
                        }
                    }
                }
            }
        }

        sps.Log2MaxFrameNumMinus4 = (uint8_t)a_bitstream->ue();
        sps.PICOrderCNTType = a_bitstream->ue();

        switch (sps.PICOrderCNTType)
        {
        case 0:
        {
            sps.Log2MaxPicOrderCNTLSBMinus4 = (uint8_t)a_bitstream->ue();

            break;
        }
        case 1:
        {
            const uint8_t deltaPIC = a_bitstream->u1();
            if (deltaPIC)
            {
                sps.Flags |= SPSFlags_DeltaPICOrderAlwaysZero;
            }

            sps.OffsetForNonRefPic = a_bitstream->se();
            sps.OffsetForTopToBottomField = a_bitstream->se();
            sps.NumRefFramesInPICOrderCNTCycle = (uint8_t)a_bitstream->ue();

            for (uint32_t i = 0; i < sps.NumRefFramesInPICOrderCNTCycle; ++i)
            {
                sps.OffsetForRefFrame[i] = a_bitstream->se();
            }

            break;
        }
        }

        sps.NumRefFrames = (uint8_t)a_bitstream->ue();
        const uint8_t gapsInFrame = a_bitstream->u1();
        if (gapsInFrame)
        {
            sps.Flags |= SPSFlags_GapsInFrameNumValueAllowed;
        }

        sps.PICWidthInMBSMinus1 = a_bitstream->ue();
        sps.PICHeightInMapUnitsMinus1 = a_bitstream->ue();

        const uint8_t frameMBS = a_bitstream->u1();
        if (frameMBS)
        {
            sps.Flags |= SPSFlags_FrameMBSOnly;
        }
        else
        {
            const uint8_t mBAdaptive = a_bitstream->u1();
            if (mBAdaptive)
            {
                sps.Flags |= SPSFlags_MBAdaptiveFrameField;
            }
        }

        const uint8_t direct8x8 = a_bitstream->u1();
        if (direct8x8)
        {
            sps.Flags |= SPSFlags_Direct8x8Inference;
        }

        const uint8_t frameCropping = a_bitstream->u1();
        if (frameCropping)
        {
            sps.Flags |= SPSFlags_FrameCropping;

            sps.FrameCropLeftOffset = a_bitstream->ue();
            sps.FrameCropRightOffset = a_bitstream->ue();
            sps.FrameCropTopOffset = a_bitstream->ue();
            sps.FrameCropBottomOffset = a_bitstream->ue();
        }

        const uint8_t vuiParams = a_bitstream->u1();
        if (vuiParams)
        {
            sps.Flags |= SPSFlags_VUIParametersPresent;

            sps.VUI = ReadVUI(a_bitstream);
        }

        a_bitstream->Ignore(1);

        a_bitstream->Ignore(a_bitstream->BitsLeft);

        return sps;
    }

    static bool MoreRBSPData(BitStream* a_stream)
    {
        if (a_stream->IsEOF())
        {
            return false;
        }

        BitStream tmp = *a_stream;
        if (tmp.u1() == 0)
        {
            return true;
        }

        while (!tmp.IsEOF()) 
        {
            if (tmp.u1())
            {
                return true;
            }
        }

        return false;
    }

    PPS ReadPPS(BitStream* a_bitstream)
    {
        IVERIFY(a_bitstream != nullptr);

        PPS pps = { 0 };

        pps.PICParameterSetID = a_bitstream->ue();
        pps.SEQParameterSetID = a_bitstream->ue();

        const uint8_t entropy = a_bitstream->u1();
        if (entropy)
        {
            pps.Flags |= PPSFlags_EntropyCodingMode;
        }

        const uint8_t picOrder = a_bitstream->u1();
        if (picOrder)
        {
            pps.Flags |= PPSFlags_PICOrderPresent;
        }

        pps.NumSliceGroupsMinus1 = a_bitstream->ue();
        if (pps.NumSliceGroupsMinus1 > 0)
        {
            pps.SliceGroupMapType = a_bitstream->ue();
            switch (pps.SliceGroupMapType) 
            {
            case 0:
            {
                for (uint32_t i = 0; i <= pps.NumSliceGroupsMinus1; ++i)
                {
                    pps.RunLengthMinus1[i] = a_bitstream->ue();
                }

                break;
            }
            case 2:
            {
                for (uint32_t i = 0; i < pps.NumSliceGroupsMinus1; ++i)
                {
                    pps.TopLeft[i] = a_bitstream->ue();
                    pps.BottomRight[i] = a_bitstream->ue();
                }

                break;
            }
            case 3:
            case 4:
            case 5:
            {
                const uint8_t changeDirection = a_bitstream->u1();
                if (changeDirection)
                {
                    pps.Flags |= PPSFlags_SliceGroupChangeDirection;
                }

                pps.SliceGroupChangeRateMinus1 = a_bitstream->ue();

                break;
            }
            case 6:
            {
                pps.PICSizeInMapUnitsMinus1 = a_bitstream->ue();

                const uint32_t v = glm::log2(pps.NumSliceGroupsMinus1 + 1);

                for (uint32_t i = 0; i <= pps.PICSizeInMapUnitsMinus1; ++i)
                {
                    pps.SliceGroupID[i] = a_bitstream->u(v);
                }

                break;
            }
            }
        }

        pps.NumRefIdxl0ActiveMinus1 = a_bitstream->ue();
        pps.NumRefIdxl1ActiveMinus1 = a_bitstream->ue();

        const uint8_t weightedPred = a_bitstream->u1();
        if (weightedPred)
        {
            pps.Flags |= PPSFlags_WeightedPred;
        }

        pps.WeightedBipredIDC = a_bitstream->u<uint8_t>(2);
        pps.PICInitQPMinus26 = a_bitstream->se();
        pps.PICInitQSMinus26 = a_bitstream->se();
        pps.ChromaQPIndexOffset = a_bitstream->se();

        const uint8_t deblockingFilter = a_bitstream->u1();
        if (deblockingFilter)
        {
            pps.Flags |= PPSFlags_DeblockingFilterControlPresent;
        }

        const uint8_t constrainedIntraPred = a_bitstream->u1();
        if (constrainedIntraPred)
        {
            pps.Flags |= PPSFlags_ConstrainedIntraPred;
        }

        const uint8_t redundantPIC = a_bitstream->u1();
        if (redundantPIC)
        {
            pps.Flags |= PPSFlags_RedundantPICCNTPresentFlag;
        }

        if (MoreRBSPData(a_bitstream))
        {
            const uint8_t transform8x8 = a_bitstream->u1();
            if (transform8x8)
            {
                pps.Flags |= PPSFlags_Transform8x8ModeFlag;
            }

            const uint8_t picScaleMatrix = a_bitstream->u1();
            if (picScaleMatrix)
            {
                pps.Flags |= PPSFlags_PICScalingMatrixPresent;

                const uint32_t count = 6 + 2 * transform8x8;
                for (uint32_t i = 0; i < count; ++i)
                {
                    ISETBIT(pps.PICScalingListPresentFlag, i);

                    if (i < 6)
                    {
                        ReadScalingList(a_bitstream, pps.ScalingList4x4[i], 16, &pps.UseDefaultScalingMatrixFlag, i);
                    }
                    else 
                    {
                        ReadScalingList(a_bitstream, pps.ScalingList8x8[i], 64, &pps.UseDefaultScalingMatrixFlag, i);
                    }
                }
            }

            pps.SecondChromaQPIndexOffset = a_bitstream->se();
        }

        a_bitstream->Ignore(1);

        a_bitstream->Ignore(a_bitstream->BitsLeft);

        return pps;
    }

    static bool IsSliceType(e_SliceHeaderType a_lhs, e_SliceHeaderType a_rhs)
    {
        uint32_t lVal = a_lhs;
        uint32_t rVal = a_rhs;
        if (lVal >= 5) 
        {
            lVal -= 5;
        }
        if (rVal >= 5)
        {
            rVal -= 5;
        }

        return lVal == rVal;
    }

    static SliceReorder ReadSliceReorder(BitStream* a_bitstream)
    {
        SliceReorder reorder = { 0 };

        uint32_t n = 0;
        do
        {
            const uint32_t idcVal = a_bitstream->ue();
            reorder.ModificationOfPICNumsIDC[n] = idcVal;
            switch (idcVal) 
            {
            case 0:
            case 1:
            {
                reorder.AbsDiffPICNumMinus1[n] = a_bitstream->ue();

                break;
            }
            case 2:
            {
                reorder.LongTermPICNum[n] = a_bitstream->ue();

                break;
            }
            }
        }
        while (reorder.ModificationOfPICNumsIDC[n++] != 3 && !a_bitstream->IsEOF());

        return reorder;
    }

    static SliceRefPICListReorder ReadRefPICListReorder(BitStream* a_bitstream, e_SliceHeaderType a_type)
    {
        SliceRefPICListReorder refPic = { 0 };

        uint32_t sliceInt = a_type;
        if (sliceInt >= 5)
        {
            sliceInt -= 5;
        }

        if (sliceInt != SliceHeaderType_I && sliceInt != SliceHeaderType_SI)
        {
            const uint8_t refPICList = a_bitstream->u1();
            if (refPICList)
            {
                ISETBIT(refPic.RefPICListReorderingFlags, 0);

                refPic.Reorderl0 = ReadSliceReorder(a_bitstream);
            }
        }

        if (sliceInt == SliceHeaderType_B)
        {
            const uint8_t refPICList = a_bitstream->u1();
            if (refPICList)
            {
                ISETBIT(refPic.RefPICListReorderingFlags, 1);

                refPic.Reorderl1 = ReadSliceReorder(a_bitstream);
            }
        }

        return refPic;
    }

    static SlicePredictiveWeightTable ReadPredictiveWeightTable(BitStream* a_bitStream, const SPS& a_sps, const PPS& a_pps, e_SliceHeaderType a_type)
    {
        SlicePredictiveWeightTable table = { 0 };
        
        table.LumaLog2WeightDenom = (uint8_t)a_bitStream->ue();
        if (a_sps.ChromaFormatIDC != 0)
        {
            table.ChromaLog2WeightDenom = (uint8_t)a_bitStream->ue();
        }

        for (uint32_t i = 0; i <= a_pps.NumRefIdxl0ActiveMinus1; ++i)
        {
            const uint32_t flagIndex = i / 8;
            const uint32_t flagOffset = i % 8;

            const uint8_t lumaWeightFlag = a_bitStream->u1();
            if (lumaWeightFlag)
            {
                ISETBIT(table.LumaWeightl0Flag[flagIndex], flagOffset);

                table.LumaWeightl0[i] = (int8_t)a_bitStream->se();
                table.LumaOffsetl0[i] = (int8_t)a_bitStream->se();
            }

            if (a_sps.ChromaFormatIDC != 0)
            {
                const uint8_t chromaWeightFlag = a_bitStream->u1();
                if (chromaWeightFlag)
                {
                    ISETBIT(table.ChromaWeightl0Flag[flagIndex], flagOffset);

                    for (uint32_t j = 0; j < 2; ++j)
                    {
                        table.ChromaWeightl0[i][j] = (int8_t)a_bitStream->se();
                        table.ChromaOffsetl0[i][j] = (int8_t)a_bitStream->se();
                    }
                }
            }
        }

        uint32_t sliceInt = a_type;
        if (sliceInt >= 5)
        {
            sliceInt -= 5;
        }

        if (sliceInt == SliceHeaderType_B)
        {
            for (uint32_t i = 0; i <= a_pps.NumRefIdxl1ActiveMinus1; ++i)
            {
                const uint32_t flagIndex = i / 8;
                const uint32_t flagOffset = i % 8;

                const uint8_t lumaWeightFlag = a_bitStream->u1();
                if (lumaWeightFlag)
                {
                    ISETBIT(table.LumaWeightl1Flag[flagIndex], flagOffset);

                    table.LumaWeightl1[i] = (int8_t)a_bitStream->se();
                    table.LumaOffsetl1[i] = (int8_t)a_bitStream->se();
                }

                if (a_sps.ChromaFormatIDC != 0)
                {
                    const uint8_t chromaWeightFlag = a_bitStream->u1();
                    if (chromaWeightFlag)
                    {
                        ISETBIT(table.ChromaWeightl1Flag[flagIndex], flagOffset);

                        for (uint32_t j = 0; j < 2; ++j)
                        {
                            table.ChromaWeightl1[i][j] = (int8_t)a_bitStream->se();
                            table.ChromaOffsetl1[i][j] = (int8_t)a_bitStream->se();
                        }
                    }
                }
            }
        }

        return table;
    }

    static SliceDecodedRefPicMarking ReadDecodedRefPicMarking(BitStream* a_bitStream, const NALHeader& a_nal)
    {
        SliceDecodedRefPicMarking marking = { 0 };

        if (a_nal.Type == NALUnitType_CodedSliceIDR)
        {
            const uint8_t noOutput = a_bitStream->u1();
            if (noOutput)
            {
                marking.Flags |= DecodedRefPicMarkingFlags_NoOutputOfPriorPICs;
            }
            const uint8_t longTerm = a_bitStream->u1();
            if (longTerm)
            {
                marking.Flags |= DecodedRefPicMarkingFlags_LongTermReference;
            }
        }
        else
        {
            const uint8_t adaptive = a_bitStream->u1();
            if (adaptive)
            {
                marking.Flags |= DecodedRefPicMarkingFlags_AdaptiveRefPICMarkingMode;

                uint32_t n = 0;
                do
                {
                    const uint8_t controlMode = (uint8_t)a_bitStream->ue();

                    marking.MemoryManagementControlOperation[n] = controlMode;
                    
                    if (controlMode == 1 || controlMode == 3)
                    {
                        marking.DifferenceOfPICNumsMinus1[n] = a_bitStream->ue();
                    }

                    if (controlMode == 2)
                    {
                        marking.LongTermPICNum[n] = a_bitStream->ue();
                    }

                    if (controlMode == 3 || controlMode == 6)
                    {
                        marking.LongTermFrameIdx[n] = a_bitStream->ue();
                    }

                    if (controlMode == 4)
                    {
                        marking.MaxLongTermFrameIdxPlus1[n] = a_bitStream->ue();
                    }
                }
                while(marking.MemoryManagementControlOperation[n++] != 0 && !a_bitStream->IsEOF());
            }
        }

        return marking;
    }

    SliceHeader ReadSliceHeader(BitStream* a_bitStream, const NALHeader& a_nal, const Array<PPS>& a_pps, const Array<SPS>& a_sps)
    {
        SliceHeader header = { 0 };

        header.FirstMBInSlice = a_bitStream->ue();
        header.SliceType = (e_SliceHeaderType)a_bitStream->ue();
        header.PICParameterSetID = a_bitStream->ue();

        const PPS& pps = a_pps[header.PICParameterSetID];
        const SPS& sps = a_sps[pps.SEQParameterSetID];

        header.FrameNum = a_bitStream->u(sps.Log2MaxFrameNumMinus4 + 4);

        if (sps.Flags & SPSFlags_FrameMBSOnly)
        {
            const uint8_t fieldPIC = a_bitStream->u1();
            if (fieldPIC)
            {
                header.Flags |= SliceHeaderFlags_FieldPIC;

                const uint8_t bottomField = a_bitStream->u1();
                if (bottomField)
                {
                    header.Flags |= SliceHeaderFlags_BottomField;
                }
            }
        }

        if (a_nal.Type == NALUnitType_CodedSliceIDR)
        {
            header.IDRPICID = a_bitStream->ue();
        }

        if (sps.PICOrderCNTType == 0)
        {
            header.PICOrderCNTLSB = a_bitStream->u(sps.Log2MaxPicOrderCNTLSBMinus4 + 4);

            if (pps.Flags & PPSFlags_PICOrderPresent && (header.Flags & SliceHeaderFlags_FieldPIC) == 0)
            {
                header.DeltaPICOrderCNTBottom = a_bitStream->se();
            }
        }
        else if (sps.PICOrderCNTType == 1 && (sps.Flags & SPSFlags_DeltaPICOrderAlwaysZero) == 0)
        {
            header.DeltaPICOrderCNT[0] = a_bitStream->se();
            if (pps.Flags & PPSFlags_PICOrderPresent && (header.Flags & SliceHeaderFlags_FieldPIC) == 0)
            {
                header.DeltaPICOrderCNT[1] = a_bitStream->se();
            }
        }

        if (pps.Flags & PPSFlags_RedundantPICCNTPresentFlag)
        {
            header.RedundantPICCNT = a_bitStream->ue();
        }

        uint32_t sliceInt = header.SliceType;
        if (sliceInt >= 5)
        {
            sliceInt -= 5;
        }

        if (sliceInt == SliceHeaderType_B)
        {
            const uint8_t direct = a_bitStream->u1();
            if (direct)
            {
                header.Flags |= SliceHeaderFlags_DirectSpatialMVPred;
            }
        }

        switch (sliceInt) 
        {
        case SliceHeaderType_P:
        case SliceHeaderType_SP:
        case SliceHeaderType_B:
        {
            const uint8_t numRef = a_bitStream->u1();
            if (numRef)
            {
                header.Flags |= SliceHeaderFlags_NumRefIdxActiveOverride;

                header.NumRefIdxl0ActiveMinus1 = a_bitStream->ue();

                if (sliceInt == SliceHeaderType_B)
                {
                    header.NumRefIdxl1ActiveMinus1 = a_bitStream->ue();
                }
            }

            break;
        }
        default:
        {
            break;
        }
        }

        header.RPLR = ReadRefPICListReorder(a_bitStream, header.SliceType);

        const bool pred = (pps.Flags & PPSFlags_WeightedPred) != 0 && (sliceInt == SliceHeaderType_P || sliceInt == SliceHeaderType_SP);
        const bool biPred = pps.WeightedBipredIDC == 1 && sliceInt == SliceHeaderType_B;
        if (pred || biPred)
        {
            header.PWT = ReadPredictiveWeightTable(a_bitStream, sps, pps, header.SliceType);
        }

        if (a_nal.IDC != NALRefIDC_PriorityDisposable)
        {
            header.DRPM = ReadDecodedRefPicMarking(a_bitStream, a_nal);
        }

        if ((pps.Flags & PPSFlags_EntropyCodingMode) != 0 && sliceInt != SliceHeaderType_I && sliceInt != SliceHeaderType_SI)
        {
            header.CabacInitIDC = a_bitStream->ue();
        }

        header.SliceQPDelta = a_bitStream->se();

        if (sliceInt == SliceHeaderType_SP || sliceInt == SliceHeaderType_SI)
        {
            if (sliceInt == SliceHeaderType_SP)
            {
                const uint8_t spSwitch = a_bitStream->u1();
                if (spSwitch)
                {
                    header.Flags |= SliceHeaderFlags_SPForSwitch;
                }
            }

            header.SliceQSDelta = a_bitStream->se();
        }

        if (pps.Flags & PPSFlags_DeblockingFilterControlPresent)
        {
            const uint32_t val = a_bitStream->ue();

            header.DisableDeblockingFilterIDC = val;
            if (val != 1)
            {
                header.SliceAlphaC0OffsetDiv2 = a_bitStream->se();
                header.SliceBetaOffsetDiv2 = a_bitStream->se();
            }
        }

        if (pps.NumSliceGroupsMinus1 > 0 && pps.SliceGroupMapType >= 3 && pps.SliceGroupMapType <= 5)
        {
            const uint32_t log = glm::log2(pps.PICSizeInMapUnitsMinus1 + pps.SliceGroupChangeRateMinus1 + 1);
            header.SliceGroupChangeCycle = a_bitStream->u(log);
        }

        return header;
    }
}