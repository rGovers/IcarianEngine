#pragma once

#include "DataTypes/Array.h"
#include <cstdint>

// I could not make head or tails until completely refactoring 
// Even then still goes over my head
// Credit: https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/Utility/h264.h
namespace H264
{
	static constexpr uint8_t NALStartCode[] = { 0, 0, 1 };
	static constexpr uint32_t NALStartCodeSize = sizeof(NALStartCode);

    struct BitStream
    {
        const uint8_t* Start;
        const uint8_t* End;
        const uint8_t* P;
    	int32_t BitsLeft;

        constexpr BitStream(const uint8_t* a_start, uint64_t a_size) :
            Start(a_start),
            End(a_start + a_size),
            P(a_start),
            BitsLeft(8)
        {

        }

        constexpr bool IsEOF() const
        {
            return P >= End;
        }

		constexpr void Ignore(uint64_t a_n)
		{
			P += a_n / 8;
			BitsLeft -= (int32_t)(a_n % 8);
			if (BitsLeft <= 0)
			{
				++P;
				BitsLeft = 8;
			}
		}

        template<typename T = uint8_t>
        constexpr T u1()
        {
            if (!IsEOF())
            {
                const T r = ((*P) >> --BitsLeft) & 0b1;

                if (BitsLeft <= 0)
                {
                    ++P;
                    BitsLeft = 8;
                }

                return r;
            }

            return 0;
        }

        template<typename T = uint32_t>
        constexpr T u(uint32_t a_n)
        {
            const T n1 = a_n - 1;

            T r = 0;
            for (uint32_t i = 0; i < a_n; ++i)
            {
                r |= u1<T>() << (n1 - i);
            }

            return r;
        }

        constexpr uint32_t ue()
        {
            uint32_t i = 0;
            while (u1() == 0 && i < 32 && !IsEOF())
            {
                ++i;
            }

            const uint32_t r = u(i);
            return r + (1 << i) - 1;
        }

		constexpr int32_t se()
		{
			const uint32_t r = ue();
			if (r & 0b1)
			{
				return (int32_t)(r + 1) / 2;
			}

			return (int32_t)(r / 2);
		}
    };

    enum e_NALRefIDC
    {
		NALRefIDC_PriorityDisposable = 0,
		NALRefIDC_PriorityLow = 1,
		NALRefIDC_PriorityHigh = 2,
        NALRefIDC_PriorityHighest = 3,
    };

    enum e_NALUnitType
	{
		NALUnitType_Unspecified = 0,	// Unspecified
		NALUnitType_CodedSliceNonIDR = 1,	// Coded slice of a non-IDR picture
		NALUnitType_CodedSliceDataPartitionA = 2,	// Coded slice data partition A
		NALUnitType_CodedSliceDataPartitionB = 3,	// Coded slice data partition B
		NALUnitType_CodedSliceDataPartitionC = 4,	// Coded slice data partition C
		NALUnitType_CodedSliceIDR = 5,	// Coded slice of an IDR picture
		NALUnitType_SEI = 6,	// Supplemental enhancement information (SEI)
		NALUnitType_SPS = 7,	// Sequence parameter set
		NALUnitType_PPS = 8,	// Picture parameter set
		NALUnitType_AUD = 9,	// Access unit delimiter
		NALUnitType_EndOfSequence = 10,	// End of sequence
		NALUnitType_EndOfStream = 11,	// End of stream
		NALUnitType_Filler = 12,	// Filler data
		NALUnitType_SPSExt = 13,	// Sequence parameter set extension
		NALUnitType_CodedSliceAUX = 19,	// Coded slice of an auxiliary coded picture without partitioning
	};

    struct NALHeader
    {
        e_NALRefIDC IDC;
        e_NALUnitType Type;
    };

	struct VUIHRD
    {
        uint32_t CPBCNTMinus1;
		uint32_t BitRateValueMinus1[32];
		uint32_t CPBSizeValueMinus1[32];

		uint32_t CBRFlag;

		uint8_t BitRateScale;
		uint8_t CPBSizeScale;
		uint8_t InitialCPBRemovalDelayLengthMinus1;
		uint8_t CPBRemovalDelayLengthMinus1;
		uint8_t DPBOutputDelayLengthMinus1;
		uint8_t TimeOffsetLength;
    };

	enum e_SPSVUIFlags : uint16_t
	{
		SPSVUIFlags_AspectRatioInfoPresent = 0b1 << 0,
		SPSVUIFlags_OverscanInfoPresent = 0b1 << 1,
		SPSVUIFlags_OverscanAppropriate = 0b1 << 2,
		SPSVUIFlags_VideoSignalTypePresent = 0b1 << 3,
		SPSVUIFlags_VideoFullRange = 0b1 << 4,
		SPSVUIFlags_ColourDescriptionPresent = 0b1 << 5,
		SPSVUIFlags_ChromaLOCInfoPresent = 0b1 << 6,
		SPSVUIFlags_TimingInfoPresent = 0b1 << 7,
		SPSVUIFlags_FixedFrameRate = 0b1 << 8,
		SPSVUIFlags_NALHRDParametersPresent = 0b1 << 9,
		SPSVUIFlags_VCLHRDParametersPresent = 0b1 << 10,
		SPSVUIFlags_LowDelayHRD = 0b1 << 11,
		SPSVUIFlags_PICStructPresent = 0b1 << 12,
		SPSVUIFlags_BitstreamRestriction = 0b1 << 13,
		SPSVUIFlags_MotionVectorsOverPICBoundaries = 0b1 << 14
	};

    struct SPSVUI
	{
		uint32_t ChromaSampleLOCTypeTopField;
		uint32_t ChromaSampleLOCTypeBottomField;
		uint32_t NumUnitsInTick;
		uint32_t TimeScale;
		uint32_t MaxBytesPerPICDenom;
		uint32_t MaxBitsPerMBDenom;
		uint32_t Log2MaxMvLengthHorizontal;
		uint32_t Log2MaxMvLengthVertical;

		VUIHRD NALHRD;
		VUIHRD VCLHRD;

		uint16_t SARWidth;
		uint16_t SARHeight;

		uint16_t Flags;

		uint8_t AspectRatioIDC;
		uint8_t VideoFormat;
		uint8_t ColourPrimaries;
		uint8_t TransferCharacteristics;
		uint8_t MatrixCoefficients;
		uint8_t NumReorderFrames;
		uint8_t MaxDecFrameBuffering;
	};

	enum e_SPSFlags : uint16_t
	{
		SPSFlags_SeparateColourPlane = 0b1 << 0,
		SPSFlags_QPPrimeYZeroTransformBypass = 0b1 << 1,
		SPSFlags_SeqScalingMatrixPresent = 0b1 << 2,
		SPSFlags_DeltaPICOrderAlwaysZero = 0b1 << 3,
		SPSFlags_GapsInFrameNumValueAllowed = 0b1 << 4,
		SPSFlags_FrameMBSOnly = 0b1 << 5,
		SPSFlags_MBAdaptiveFrameField = 0b1 << 6,
		SPSFlags_Direct8x8Inference = 0b1 << 7,
		SPSFlags_FrameCropping = 0b1 << 8,
		SPSFlags_VUIParametersPresent = 0b1 << 9
	};

    struct SPS
    {
		uint32_t PICOrderCNTType;
		int32_t OffsetForNonRefPic;
		int32_t OffsetForTopToBottomField;
		int32_t OffsetForRefFrame[256];
		uint32_t PICWidthInMBSMinus1;
		uint32_t PICHeightInMapUnitsMinus1;
		uint32_t FrameCropLeftOffset;
		uint32_t FrameCropRightOffset;
		uint32_t FrameCropTopOffset;
		uint32_t FrameCropBottomOffset;

        SPSVUI VUI;

		uint16_t Flags;

		uint8_t NumRefFramesInPICOrderCNTCycle;
		uint8_t ProfileIDC;
		uint8_t LevelIDC;
		uint8_t ChromaFormatIDC;
		uint8_t SeqParameterSetID;
		uint8_t BitDepthLumaMinus8;
		uint8_t BitDepthChromaMinus8;
		uint8_t Log2MaxPicOrderCNTLSBMinus4;
		int8_t ScalingList4x4[6][16];
		int8_t ScalingList8x8[2][64];
		uint8_t Log2MaxFrameNumMinus4;
		uint8_t NumRefFrames;

        uint8_t ConstraintSetFlag;
		uint8_t SeqScalingListPresentFlag;
		uint8_t UseDefaultScalingMatrixFlag;
	};

	enum e_PPSFlags : uint16_t
	{
		PPSFlags_EntropyCodingMode = 0b1 << 0,
		PPSFlags_PICOrderPresent = 0b1 << 1,
		PPSFlags_SliceGroupChangeDirection = 0b1 << 2,
		PPSFlags_WeightedPred = 0b1 << 3,
		PPSFlags_DeblockingFilterControlPresent = 0b1 << 4,
		PPSFlags_ConstrainedIntraPred = 0b1 << 5,
		PPSFlags_RedundantPICCNTPresentFlag = 0b1 << 6,
		PPSFlags_Transform8x8ModeFlag = 0b1 << 7,
		PPSFlags_PICScalingMatrixPresent = 0b1 << 8
	};

	struct PPS
	{
		uint32_t PICParameterSetID;
		uint32_t SEQParameterSetID;
		uint32_t NumSliceGroupsMinus1;
		uint32_t SliceGroupMapType;
		uint32_t RunLengthMinus1[8];
		uint32_t TopLeft[8];
		uint32_t BottomRight[8];
		uint32_t SliceGroupChangeRateMinus1;
		uint32_t PICSizeInMapUnitsMinus1;
		uint32_t SliceGroupID[256];
		uint32_t NumRefIdxl0ActiveMinus1;
		uint32_t NumRefIdxl1ActiveMinus1;
		int32_t PICInitQPMinus26;
		int32_t PICInitQSMinus26;
		int32_t ChromaQPIndexOffset;

		int8_t ScalingList4x4[6][16];
		int8_t ScalingList8x8[2][64];
		int32_t SecondChromaQPIndexOffset;

		uint16_t Flags;

		uint8_t WeightedBipredIDC;
		uint8_t PICScalingListPresentFlag;
		uint8_t UseDefaultScalingMatrixFlag;
	};

	struct SlicePredictiveWeightTable
	{
		uint8_t LumaLog2WeightDenom;
		uint8_t ChromaLog2WeightDenom;
		int8_t LumaWeightl0[64];
		int8_t LumaOffsetl0[64];
		int8_t ChromaWeightl0[64][2];
		int8_t ChromaOffsetl0[64][2];
		int8_t LumaWeightl1[64];
		int8_t LumaOffsetl1[64];
		int8_t ChromaWeightl1[64][2];
		int8_t ChromaOffsetl1[64][2];

		uint8_t LumaWeightl0Flag[8];
		uint8_t ChromaWeightl0Flag[8];
		uint8_t LumaWeightl1Flag[8];
		uint8_t ChromaWeightl1Flag[8];
	};

	struct SliceReorder
	{
		uint8_t ModificationOfPICNumsIDC[64];
		int32_t AbsDiffPICNumMinus1[64];
		int32_t LongTermPICNum[64];
	};

	struct SliceRefPICListReorder
	{
		SliceReorder Reorderl0;
		SliceReorder Reorderl1;
		uint8_t RefPICListReorderingFlags;
	};

	enum e_DecodedRefPicMarkingFlags : uint16_t
	{
		DecodedRefPicMarkingFlags_NoOutputOfPriorPICs = 0b1 << 0,
		DecodedRefPicMarkingFlags_LongTermReference = 0b1 << 1,
		DecodedRefPicMarkingFlags_AdaptiveRefPICMarkingMode = 0b1 << 2,
	};

	struct SliceDecodedRefPicMarking
	{
		uint8_t MemoryManagementControlOperation[64];
		uint32_t DifferenceOfPICNumsMinus1[64];
		uint32_t LongTermPICNum[64];
		uint32_t LongTermFrameIdx[64];
		uint32_t MaxLongTermFrameIdxPlus1[64];

		uint16_t Flags;
	};

	enum e_SliceHeaderFlags
	{
		SliceHeaderFlags_FieldPIC = 0b1 << 0,
		SliceHeaderFlags_BottomField = 0b1 << 1,
		SliceHeaderFlags_DirectSpatialMVPred = 0b1 << 2,
		SliceHeaderFlags_NumRefIdxActiveOverride = 0b1 << 3,
		SliceHeaderFlags_SPForSwitch = 0b1 << 4
	};

	enum e_SliceHeaderType
	{
		SliceHeaderType_P = 0,
		SliceHeaderType_B = 1,
		SliceHeaderType_I = 2,
		SliceHeaderType_SP = 3,
		SliceHeaderType_SI = 4,
		SliceHeaderType_POnly = 5,
		SliceHeaderType_BOnly = 6,
		SliceHeaderType_IOnly = 7,
		SliceHeaderType_SPOnly = 8,
		SliceHeaderType_SIOnly = 9,
	};

	struct SliceHeader
	{
		uint32_t FirstMBInSlice;
		e_SliceHeaderType SliceType;
		uint32_t PICParameterSetID;
		uint32_t FrameNum;
		uint32_t IDRPICID;
		uint32_t PICOrderCNTLSB;
		int32_t DeltaPICOrderCNTBottom;
		int32_t DeltaPICOrderCNT[2];
		uint32_t RedundantPICCNT;
		uint32_t NumRefIdxl0ActiveMinus1;
		uint32_t NumRefIdxl1ActiveMinus1;
		uint32_t CabacInitIDC;
		int32_t SliceQPDelta;
		int32_t SliceQSDelta;
		uint32_t DisableDeblockingFilterIDC;
		int32_t SliceAlphaC0OffsetDiv2;
		int32_t SliceBetaOffsetDiv2;
		uint32_t SliceGroupChangeCycle;

		SlicePredictiveWeightTable PWT;
		SliceRefPICListReorder RPLR;
		SliceDecodedRefPicMarking DRPM;

		uint16_t Flags;
	};

    NALHeader ReadNALHeader(BitStream* a_bitstream);
    SPS ReadSPS(BitStream* a_bitstream);
	PPS ReadPPS(BitStream* a_bitstream);
	SliceHeader ReadSliceHeader(BitStream* a_bitStream, const NALHeader& a_nal, const Array<PPS>& a_pps, const Array<SPS>& a_sps);
};