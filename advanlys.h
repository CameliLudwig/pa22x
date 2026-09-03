/*============================================================================*/
/*                        L a b W i n d o w s / C V I                         */
/*----------------------------------------------------------------------------*/
/*    Copyright (c) National Instruments 1987-2020.  All Rights Reserved.     */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Title:       analysis.h                                                    */
/* Purpose:     Include file for LabWindows CVI Advanced Analysis Library     */
/*                                                                            */
/*============================================================================*/


#ifndef _ANALYSIS_H
#define _ANALYSIS_H

#if defined(_CVI_) && !defined(__TPC__)
#pragma EnableLibraryRuntimeChecking
#endif

#include "cvidef.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Error Codes */
typedef enum {
    NoAnlysErr              =      0,	// No Error.
    OutOfMemAnlysErr        = -20001,	// Out of memory error.
    EqSamplesAnlysErr       = -20002,	// Input sequences must be the same size.
    SamplesGTZeroAnlysErr   = -20003,	// Number of samples must be greater than zero.
    SamplesGEZeroAnlysErr   = -20004,	// Number of samples must be greater than or equal to zero.
    SamplesGEOneAnlysErr    = -20005,	// The number of samples must be greater than or equal to one.
    SamplesGETwoAnlysErr    = -20006,	// Number of samples must be greater than or equal to two.
    SamplesGEThreeAnlysErr  = -20007,	// Number of samples must be greater than or equal to three.
    ArraySizeAnlysErr       = -20008,	// Specified conditions on the input arrays have not been met.
    PowerOfTwoAnlysErr      = -20009,	// Size of the input array must be a valid power of two: size = 2m.
    MaxXformSizeAnlysErr    = -20010,	// The maximum allowable transform size has been exceeded.
    DutyCycleAnlysErr       = -20011,	// The duty cycle must meet: 0 <= duty cycle <= 100.
    CyclesAnlysErr          = -20012,	// Number of cycles must meet the condition 0 < cycles <= samples.
    WidthLTSamplesAnlysErr  = -20013,	// The width must meet: 0 < width < samples.
    DelayWidthAnlysErr      = -20014,	// Delay and width must meet the condition 0 <= (delay + width) < samples.
    DtGEZeroAnlysErr        = -20015,	// dt must be greater than or equal to zero.
    DtGTZeroAnlysErr        = -20016,	// dt or dx must be greater than zero.
    IndexLTSamplesAnlysErr  = -20017,	// Index must meet the condition 0 <= index < samples.
    IndexLengthAnlysErr     = -20018,	// The index and length must meet the condition 0 <= (index + length) < samples.
    UpperGELowerAnlysErr    = -20019,	// Upper value must be greater than or equal to the lower value.
    NyquistAnlysErr         = -20020,	// Cut-off frequency fc must meet the condition 0 <= fc <= (fs/2).
    OrderGTZeroAnlysErr     = -20021,	// Order must be greater than zero.
    DecFactAnlysErr         = -20022,	// Decimating factor must meet the condition 0 < decimating factor <= samples.
    BandSpecAnlysErr        = -20023,	// Invalid band specification.
    RippleGTZeroAnlysErr    = -20024,	// Ripple must be greater than zero.
    AttenGTZeroAnlysErr     = -20025,	// Attenuation must be greater than zero.
    WidthGTZeroAnlysErr     = -20026,	// The width must be greater than zero.
    FinalGTZeroAnlysErr     = -20027,	// The final value must be greater than zero.
    AttenGTRippleAnlysErr   = -20028,	// Attenuation must be greater than the ripple amplitude.
    StepSizeAnlysErr        = -20029,	// The step-size parameter mu must meet 0 <= mu <= 0.1.
    LeakAnlysErr            = -20030,	// The leakage coefficient Leak must meet: 0 <= Leak <= mu.
    EqRplDesignAnlysErr     = -20031,	// Filter cannot be designed with the specified input parameters.
    RankAnlysErr            = -20032,	// The rank of the filter must meet 1 <= (2 rank +1) <= size.
    EvenSizeAnlysErr        = -20033,	// Number of coefficients must be odd for this filter.
    OddSizeAnlysErr         = -20034,	// Number of coefficients must be even for this filter.
    StdDevAnlysErr          = -20035,	// The standard deviation is zero. The normalization is impossible.
    MixedSignAnlysErr       = -20036,	// Second array must be all positive or negative and nonzero.
    SizeGTOrderAnlysErr     = -20037,	// Array size must be greater than the order.
    IntervalsAnlysErr       = -20038,	// The number of intervals must be greater than zero.
    MatrixMulAnlysErr       = -20039,	// The specified matrix multiplication cannot be performed.
    SquareMatrixAnlysErr    = -20040,	// Input matrix must be a square matrix.
    SingularMatrixAnlysErr  = -20041,	// Input matrix is singular. The system of equations cannot be solved.
    LevelsAnlysErr          = -20042,	// Number of levels is outside the allowable range.
    FactorAnlysErr          = -20043,	// Level of factor is outside the allowable range.
    ObservationsAnlysErr    = -20044,	// There must be at least one observation.
    DataAnlysErr            = -20045,	// Total number of data points must be equal to product of (levels/ each factor) x (observations/ cell).
    OverflowAnlysErr        = -20046,	// There is an overflow in the calculated F-value(ANOVA Fit).
    BalanceAnlysErr         = -20047,	// Data is unbalanced.
    ModelAnlysErr           = -20048,	// Random Effect model was requested when the Fixed Effect model is required.
    DistinctAnlysErr        = -20049,	// x-values must be distinct.
    PoleAnlysErr            = -20050,	// Interpolating function has a pole at the requested value.
    ColumnAnlysErr          = -20051,	// First column in the X matrix must be all ones.
    FreedomAnlysErr         = -20052,	// The degree of freedom must be greater than zero and less than the length of the input sequence.
    ProbabilityAnlysErr     = -20053,	// Probability must meet the condition 0 < p < 1.
    InvProbAnlysErr         = -20054,	// The probability must meet the condition: 0 <= p < 1.
    CategoryAnlysErr        = -20055,	// Invalid number of categories or samples.
    TableAnlysErr           = -20056,	// Contingency table has a negative number.
    BetaFuncAnlysErr        = -20057,	// Parameter to the beta function must meet the condition 0 < p < 1.
    DimensionAnlysErr       = -20058,	// Invalid number of dimensions or dependent variables.
    NegNumAnlysErr          = -20059,	// Negative number.
    DivByZeroAnlysErr       = -20060,	// Divide by zero.
    InvSelectionAnlysErr    = -20061,	// Invalid selection.
    MaxIterAnlysErr         = -20062,	// Maximum iteration exceeded.
    PolyAnlysErr            = -20063,	// Invalid polynomial.
    InitStateAnlysErr       = -20064,	// Internal memory state not initialized correctly.
    ZeroVectorAnlysErr      = -20065,	// Elements of the vector cannot be all zero.
    IIRFilterInfoAnlysErr   = -20066,	// Information in the IIR filter structure is invalid.
    FFSRInvalidAnlysErr     = -20067,	// The input fundamental frequency or sampling rate is equal to zero.
    MatrixInfNanAnlysErr    = -20068,	// Input parameter has at least one element that is Inf or NaN or DBL_MIN or DBL_MAX.
    InvalidDoubleAnlysErr   = -20068,	// Input parameter has at least one element that is Inf or NaN or DBL_MIN or DBL_MAX.
    SamplesGEFourAnlysErr   = -20069,	// Number of samples must be greater than or equal to four.
    SameSizeAnlysErr        = -20070,	// Matrices must have the same size
    PosDefAnlysErr          = -20071,	// Input to Cholesky Decomposition was not positive definite.
    MatrixLnAnlysErr        = -20072,	// The logarithm of the input matrix can't be computed.
    CloseEgnValAnlysErr     = -20073,	// The eigenvalues can't be reordered because some eigenvalues are too close.
    EgnValChgedAnlysErr     = -20074,	// Reordering eigenvalues changed some complex eigenvalues.
    FltBuffOvflowAnlysErr   = -20075,	// The filter buffer overflows.
    RsmplBydInputAnlysErr   = -20076,	// The resample point can't be calculated with the signal behind that of input.
    TimeNotAscendAnlysErr   = -20077,	// Time points are not in ascending order.
    NotSupportedAnlysErr    = -20078,	// Functionality is not supported on this platform.
    BaseGETopAnlysErr       = -20101,	// Parameter must meet the condition: Top>Base.
    ShiftRangeAnlysErr      = -20102,	// The shifts must meet: |shifts| < samples.
    OrderGEZeroAnlysErr     = -20103,	// The order must be positive.
    InputNaNAnlysErr        = -20104,	// The inputs contain NaN.
    ZeroPolyAnlysErr        = -20111,	// Input Polynomial coefficients are all zero.
    StartIsRootAnlysErr     = -20112,	// Poly real zeros counter: start is a root.
    EndIsRootAnlysErr       = -20113,	// Poly real zeros counter: end is a root.
    StartGTEndAnlysErr      = -20114,	// Poly real zeros counter: start > end.
    FFTSizeGTZeroAnlysErr   = -20115,	// FFT size must be greater than zero.
    NoFeasibleSolAnlysErr   = -20116,	// A feasible solution was not found.
    LossOfSignificAnlysErr  = -20117,	// The computation failed due to loss of significance.
    AbsGTOneAnlysErr        = -20118,	// The input is greater than 1 or less than -1.
    SamplesNEThreeAnlysErr  = -20119,	// The number of samples is not equal to three.

    PoolSizeSetAnlysErr     = -20120,	// The size of memory pool has already been set.
    PoolEnableAnlysErr      = -20121,	// The memory pool has already been enabled.
    PoolEmptyAnlysErr       = -20122,	// The memory pool is empty.
    PoolNotEmptyAnlysErr    = -20123,	// The memory pool is not empty.
    PoolLockedAnlysErr      = -20124,	// The memory pool has been locked.
    PoolUnlockAnlysErr      = -20125,	// The memory pool cannot be unlocked.
    PoolOutOfMemAnlysErr    = -20126,	// There is not enough space left to perform the specified routine in the memory pool.

    NegZeroNumAnlysErr      = -20140,	// The number is negtive or zero.
    ZeroNumAnlysErr         = -20141,	// The number is zero.

    ScopeOffsetAnlysErr     = -20307,	// Scope offset error.
    NotEnoughCrossPointsAnlysErr = -20308,	// Not enough crosspoints and cycles meet the measurement requirement.
    ZeroAmpAnlysErr         = -20309,	// The waveform amplitude is zero.
    NotEnoughEdgesAnlysErr  = -20310,	// Not enough edge times and cycles meet the measurement requirement.
    MemFullAnlysErr         = -20311,	// There is not enough space left to perform the specified routine.
    ZeroWfmSizeAnlysErr     = -20312,	// No waveform input.
    RefLevelsAnlysErr       = -20313,	// Expected high reference level >= middle reference level >= low reference level.
    RefUnitsAnlysErr        = -20314,	// The reference unit must be either absolute(0) or percent(1).
    DeltaTimeAnlysErr       = -20315,	// The dt of waveform <= 0.0.
    PercentMethodAnlysErr   = -20316,	// The method must be either histogram(0) or peak(1) or autoselect(2).
    HistSizeAnlysErr        = -20317,	// The histogram size <= 0.
    UndefinedResultAnlysErr = -20318,	// The parameter is undefined.
    PeriodTooShortAnlysErr  = -20319,	// The period of waveform is too short.
    ResultNotAvailableAnlysErr = -20320,	// The result is not available.
    InvalidSessionAnlysErr  = -20321,	// Invalid session.
    InvalidAttributeAnlysErr= -20322,	// Invalid attribute input.
    BadNumEdgesAnlysErr     = -20323,	// Invalid number of edge times.
    DuplicateMeasIdAnlysErr = -20324,	// Duplicate measurement id.
    InvalidMeasNumAnlysErr  = -20325,	// Invalid measurement number.
    NullWfmPointerAnlysErr  = -20312,	// No waveform input.
    InfiniteSlewRateAnlysErr= -20326,	// The slope is infinite.
    ExceptionAnlysErr       = -20327,	// An exception has occurred.
    CrictSectAnlysErr       = -20328,	// Wrong section.
    NullCursorStructPtrAnlysErr = -20329,	// The cursor structure pointer is NULL.
    NullResultPtrAnlysErr   = -20330,	// The result pointer is NULL.
    NullParamPtrAnlysErr    = -20331,	// The reference levels or the percent level settings are NULL.
    InvalidNumThreadsAnlysErr = -20332,	// Invalid number of threads.

    InternalAnlysErr        = -20999,	// Internal analysis library error.
    RankDeficient           =  20001,	// The matrix is rank deficient.
    SamplesGTZeroAnlysWarn  =  20002,	// The number of samples must be greater than zero.
    SingularMatrixAnlysWarn =  20003,	// The matrix is singular.
    SameSizeAnlysWarn       =  20004,	// Matrices or vectors don't have the same size.
    ReorderAnlysWarn        =  20005,	// The reorder procedure cannot be performed.
    AccuracyAnlysWarn       =  20006,	// The computation result might be inaccurate.
    ArraySizeAnlysWarn      =  20010,	// The specified conditions on the sizes of input arrays have not been met.
    InvStdDevAnlysWarn      =  20011,	// The input Standard Deviation is invalid.
    EmptyPolyAnlysWarn      =  20012,	// The input polynomial is empty.
    MatrixInfNanAnlysWarn   =  20030,	// Inf or NaN elements found in the input.
    MaxOrderAnlysWarn       =  20031,	// The maximum allowable order has been exceeded. 

    LibraryNotFoundAnlysErr = -5093,	// The analysis library was not found.
    FunctionNotFoundAnlysErr= -5094,	// The function was not found in the analysis library.

    _unusedAnlysErr         = 0x7fffffff// Unused analysis error code.
} AnalysisLibErrType;

/* Values for Reference Units */
typedef enum {
	ABSOLUTE_LEVEL   = 0,	// Absolute Levels
	PERCENTAGE_LEVEL = 1	// Percentage of Waveform
} AnalysisRefUnits;

/* Filter types */
typedef enum {
    LOWPASS  = 0,		// Lowpass Filter
    HIGHPASS = 1,		// Highpass Filter
    BANDPASS = 2,		// Bandpass Filter
    BANDSTOP = 3		// Bandstop Filter
} AnalysisLibFilterType;

typedef enum {
	MULTIBAND      = 1,	// Multiband
	DIFFENTIATOR   = 2,	// Differentiator
	HILBERT_FILTER = 3	// Hilbert
} AnalysisLibFilterType_;

typedef enum {
    LOWPASS__    = 0,		// Lowpass Filter
    HIGHPASS__   = 1,		// Highpass Filter
    BANDPASS__   = 2,		// Bandpass Filter
    BANDSTOP__   = 3,		// Bandstop Filter
    WBLOWPASS__  = 4,		// Wideband Lowpass Filter
    WBHIGHPASS__ = 5 		// Wideband Highpass Filter
} AnalysisLibFilterType__;

/* Filter type for Parks_McClellan */
typedef enum {
    PM_MULTIBAND      = 0,	// Multiband
    PM_DIFFERENTIATOR = 1,	// Differentiator
    PM_HILBERT        = 2	// Hilbert
} AnalysisLibFilterType___;

/* Window types */
typedef enum {
	RECTANGLE		= 0,	// Rectangle
	HANNING			= 1,	// Hanning
	HAMMING			= 2,	// Hamming
	BLKHARRIS		= 3,	// Blackman-Harris
	EXBLKMAN		= 4,	// Exact Blackman
	BLKMAN			= 5,	// Blackman
	FLATTOP			= 6,	// Flat Top
	BH4TERM			= 7,	// 4-Term Blackman-Harris
	BH7TERM			= 8,	// 7-Term Blackman-Harris
	LOWSIDELB		= 9,	// Lowside LB
	BLKMANNUTTAL	= 11,	// Blackman-Nuttall
	TRIANGLE		= 30,	// Triangle
	BARTHANN		= 31,   // Barthann
	BOHMAN			= 32,   // Bohman
	PARZEN			= 33,   // Parzen
	WELCH			= 34,   // Welch
 	KAISER			= 60,	// Kaiser
	DOLCHEBYSHEV	= 61,	// Dolph-Chebyshev
	GAUSSIAN		= 62	// Gaussian
} AnalysisLibWindowType;

typedef enum {
	RECTANGLE_		= 0,	// Rectangle
	HANNING_		= 1,	// Hanning
	HAMMING_		= 2,	// Hamming
	BLKHARRIS_		= 3,	// Blackman-Harris
	EXBLKMAN_		= 4,	// Exact Blackman
	BLKMAN_			= 5,	// Blackman
	FLATTOP_		= 6,	// Flat Top
	BH4TERM_		= 7,	// 4-Term Blackman-Harris
	BH7TERM_		= 8	// 7-Term Blackman-Harris
} AnalysisLibWindowType_;

typedef enum {
	NO_WINDOW__		= 0,	// None
	HANNING__		= 1,	// Hanning
	HAMMING__		= 2,	// Hamming
	BLKHARRIS__		= 3,	// Blackman-Harris
	EXBLKMAN__		= 4,	// Exact Blackman
	BLKMAN__		= 5,	// Blackman
	FLATTOP__		= 6,	// Flat Top
	BH4TERM__		= 7,	// 4-Term Blackman-Harris
	BH7TERM__		= 8	// 7-Term Blackman-Harris
} AnalysisLibWindowType__;

typedef enum {
	RECTANGLE___	= 0,	// Rectangle
	HANNING___		= 1,	// Hanning
	HAMMING___		= 2,	// Hamming
	BLKHARRIS___	= 3,	// Blackman-Harris
	EXBLKMAN___		= 4,	// Exact Blackman
	BLKMAN___		= 5,	// Blackman
	FLATTOP___		= 6,	// Flat Top
	BH4TERM___		= 7,	// 4-Term Blackman-Harris
	BH7TERM___		= 8,	// 7-Term Blackman-Harris
	LOWSIDELB___	= 9,	// Lowside LB
	BLKMANNUTTAL___	= 11 	// Blackman-Nuttall
} AnalysisLibWindowType___;

typedef enum {
	RECTANGLE____		= 0,	// Rectangle
	HANNING____			= 1,	// Hanning
	HAMMING____			= 2,	// Hamming
	BLKHARRIS____		= 3,	// Blackman-Harris
	EXBLKMAN____		= 4,	// Exact Blackman
	BLKMAN____			= 5,	// Blackman
	FLATTOP____			= 6,	// Flat Top
	BH4TERM____			= 7,	// 4-Term Blackman-Harris
	BH7TERM____			= 8,	// 7-Term Blackman-Harris
	LOWSIDELB____		= 9,	// Lowside LB
	BLKMANNUTTAL____	= 11, 	// Blackman-Nuttall
	TRIANGLE____		= 30,	// Triangle
 	KAISER____			= 60,	// Kaiser
	DOLCHEBYSHEV____	= 61,	// Dolph-Chebyshev
	GAUSSIAN____		= 62	// Gaussian
} AnalysisLibWindowType____;

typedef enum {
	RECTANGLE_FILTER	= 1,	// rectangular
	TRIANGLE_FILTER		= 2,	// triangle
	HANNING_FILTER		= 3,	// Hanning
	HAMMING_FILTER		= 4,	// Hamming
	BLKMAN_FILTER		= 5	// Blackman
} AnalysisLibWindowFilter;

/* Eigenvectors used by EigenVBack */
typedef enum {
	RIGHT_EIGEN		= 0,	// Right Side Eigenvectors
	LEFT_EIGEN		= 1		// Left Side Eigenvectors
} AnalysisLibEigenvectorType;

/* Eigenvalues and eigenvectors */
typedef enum {
	EIGEN_VALUES_ONLY    = 0,	// Only Eigen Values
	EIGEN_VALUES_VECTORS = 1	// Both Eigen Values and Eigen Vectors
} AnalysisLibEigenValueVectors;

/* Pivot used by QREx */
typedef enum {
	NOT_PIVOT		= 0,	// No Pivot
	PIVOT_VECTOR	= 1,	// Pivot as a Vector
	PIVOT_MATRIX	= 2		// Pivot as a Matrix
} AnalysisLibPivot;

/* Options for QREx */
typedef enum {
	FULL_SIZE		= 0,	// Full Size
	ECONOMY_SIZE	= 1		// Economical Size
} AnalysisLibSizeOption;

/* Eigenvalue ordering used by Schur */
typedef enum {
	NO_ORDERING     = 0,	// No ordering
	REAL_ASCENDING	= 1,	// Real Part Ascending
	REAL_DESCENDING	= 2,	// Real Part Descending
	MAG_ASCENDING	= 3,	// Magnitude Ascending
	MAG_DESCENDING	= 4		// Magnitude Descending
} AnalysisLibEigenvalueOrdering;

/* Matrix balance */
typedef enum {
	NONE			= 0,	// Neither Permuted nor Scaled
	PERMUTED		= 1,	// Permuted
	SCALED			= 2,	// Scaled
	PERMUTED_SCALED	= 3		// Permuted and Scaled
} AnalysisLibMatrixBalance;

/* Special matrix type */
typedef enum {
	IDENTITY		= 0,	// Identity Matrix
	DIAGONAL		= 1,	// Diagonal Matrix
	TOEPLITZ		= 2,	// Toeplitz Matrix
	VANDERMONDE		= 3,	// Vandermonde Matrix
	COMPANION		= 4		// Companion Matrix
} AnalysisLibSpecialMatrixType;

/* Matrix type used by SolveEqs */
typedef enum {
	GENERAL_MATRIX = 0,		// General Matrix
	SYMPOS_MATRIX  = 1,		// Symmetric and Positive Definite Matrix
	LOTRI_MATRIX   = 2,		// Lower Triangular Matrix
	UPTRI_MATRIX   = 3		// Upper Triangular Matrix
} AnalysisLibMatrixType;

/* Matrix type for CxEigenValueVector */
typedef enum {
	COMPLEX_GENERAL_MATRIX   = 0,	// General Matrix
	COMPLEX_HERMITIAN_MATRIX = 1	// Hermitian Matrix
} AnalysisLibComplexMatrixType;

/* Matrix norm type */
typedef enum {
	NORM_TYPE_2         = 0,	// 2-Norm
	NORM_TYPE_1         = 1,	// 1-Norm
	NORM_TYPE_FROBENIUS = 2,	// Frobenius-Norm
	NORM_TYPE_INFINITE  = 3		// Infinite-Norm
} AnalysisLibNormType;

/* Fit method type used by Curve Fitting functions */
typedef enum {
	LEAST_SQUARE            = 0,	// Least square
	LEAST_ABSOLUTE_RESIDUAL = 1,	// Least absolute residual
	BISQUARE                = 2		// Bisquare
} AnalysisLibFitMethod;

/* Interval type used by Curve Fitting functions */
typedef enum {
	CONFIDENCE_INTVL = 0,			// Confidence Interval
	PREDICTION_INTVL = 1			// Prediction Interval
} AnalysisLibIntervalType;

/* Range type used by RemoveOutlier functions */
typedef enum {
	RANGE_OF_Y		 = 0,			// Range of Y
	RANGE_OF_X		 = 1,			// Range of X
	RANGE_OF_X_AND_Y = 2			// Range of X and Y
} AnalysisLibRangeType;

/* Interpolation type used by ArbitraryWave */
typedef enum {
	LINEAR_INTERPOLATION = 1,		// Linear
	NO_INTERPOLATION     = 0		// None
} AnalysisLibInterpolation;

/* Sort ordering */
typedef enum {
	ANALYSIS_SORT_DESCENDING = 1,			// descending
	ANALYSIS_SORT_ASCENDING  = 0			// ascending
} AnalysisLibSortOrdering;

/* Powers of two */
typedef enum {
	POWER_2_1	= 2,				// 2
	POWER_2_2	= 4,				// 4
	POWER_2_3	= 8,				// 8
	POWER_2_4	= 16,				// 16
	POWER_2_5	= 32,				// 32
	POWER_2_6	= 64,				// 64
	POWER_2_7	= 128,				// 128
	POWER_2_8	= 256,				// 256
	POWER_2_9	= 512,				// 512
	POWER_2_10	= 1024,				// 1024
	POWER_2_11	= 2048,				// 2048
	POWER_2_12	= 4096,				// 4096
	POWER_2_13	= 8192,				// 8192
	POWER_2_14	= 16384,			// 16384
	POWER_2_15	= 32768,			// 32768
	POWER_2_16	= 65536,			// 65536
	POWER_2_17	= 131072,			// 131072
	POWER_2_18	= 262144,			// 262144
	POWER_2_19	= 524288,			// 524288
	POWER_2_20	= 1048576,			// 1048576
	POWER_2_21	= 2097152,			// 2097152
	POWER_2_22	= 4194304,			// 4194304
	POWER_2_23	= 8388608,			// 8388608
	POWER_2_24	= 16777216,			// 16777216
	POWER_2_25	= 33554432,			// 33554432
	POWER_2_26	= 67108864,			// 67108864
	POWER_2_27	= 134217728,		// 134217728
	POWER_2_28	= 268435456			// 268435456
} AnalysisLibPowerOfTwo;

/* Enable or disable an option */
typedef enum {
	ENABLE_OPTION  = 1,				// On
	DISABLE_OPTION = 0				// Off
} AnalysisLibEnableDisable;

/* True or false */
typedef enum {
	ANALYSIS_TRUE  = 1,				// Yes
	ANALYSIS_FALSE = 0				// No
} AnalysisLibTrueFalse;

/* High or low */
typedef enum {
	ANALYSIS_HIGH  = 1,				// High
	ANALYSIS_LOW   = 0				// Low
} AnalysisLibHighLow;

/* Polarity */
typedef enum {
	DETECT_PEAKS   = 0,				// Peaks
	DETECT_VALLEYS = 1				// Valleys
} AnalysisLibPeakDetection;

/* Spectrum type */
typedef enum {
	SPECTRUM_POWER     = 0,			// power
	SPECTRUM_AMPLITUDE = 1,			// amplitude
	SPECTRUM_GAIN      = 2			// gain
} AnalysisLibSpectrumType;

/* Spectrum type */
typedef enum {
	SCALING_MODE_LINEAR   = 0,		// linear
	SCALING_MODE_DECIBELS = 1,		// dB
	SCALING_MODE_DBM      = 2		// dBm
} AnalysisLibScalingMode;

/* Display unit */
typedef enum {
	DISPLAY_UNIT_VRMS     = 0,		// rms
	DISPLAY_UNIT_VPK      = 1,		// pk
	DISPLAY_UNIT_VRMS2    = 2,		// rms^2
	DISPLAY_UNIT_VPK2     = 3,		// pk^2
	DISPLAY_UNIT_VRMSHZ   = 4,		// rms/rtHz
	DISPLAY_UNIT_VPKHZ    = 5,		// pk/rtHz
	DISPLAY_UNIT_VRMS2HZ  = 6,		// rms^2/rtHz
	DISPLAY_UNIT_VPK2HZ   = 7		// pk^2/rtHz
} AnalysisLibDisplayUnit;

/* Algorithm used to solve the multiple linear regression model */
typedef enum {
	ALGORITHM_SVD         = 0,		// SVD
	ALGORITHM_GIVENS      = 1,		// Givens
	ALGORITHM_GIVENS2     = 2,		// Givens2
	ALGORITHM_HOUSEHOLD   = 3,		// Householder
	ALGORITHM_LU_DECOMP   = 4,		// LU decomposition
	ALGORITHM_CHOLESKY    = 5		// Cholesky
} AnalysisLibGenLSFitAlgorithm;

/* Algorithm used in QR factorization */
typedef enum {
	ALGORITHM_QR_HOUSEHOLDER = 0,		// Householder
	ALGORITHM_QR_GIVENS      = 1,		// Givens
	ALGORITHM_QR_FAST_GIVENS = 2		// Fast Givens
} AnalysisLibQRAlgorithm;

/* Integration method */
typedef enum {
	TRAPEZOIDAL_RULE = 0,		// Trapezoidal Rule
	SIMPSONS_RULE    = 1,		// Simpson's Rule
	SIMPSONS_38_RULE = 2,		// Simpson's 3/8 Rule
	BODE_RULE        = 3		// Bode Rule
} AnalysisLibIntegrationMethod;

/* Algorithm used in Polynomial Fit Ex */
typedef enum {
	ALGORITHM_POLYFIT_SVD      = 0,	// SVD
	ALGORITHM_POLYFIT_QR       = 1,	// QR
	ALGORITHM_POLYFIT_LU       = 2,	// LU
	ALGORITHM_POLYFIT_CHOLESKY = 3	// Cholesky
} AnalysisLibPolyFitExAlgorithm;

/* Algorithm used in Chirp Z transform and complex Chirp Z transform */
typedef enum {
	ALGORITHM_CHIRP_DIRECT      = 0,	// Direct
	ALGORITHM_CHIRP_FREQ_DOMAIN = 1		// Frequency Domain
} AnalysisLibChirpAlgorithm;

/* Algorithm used in ConvolveEx and Correlate */
typedef enum {
	ALGORITHM_CONCOR_DIRECT      = 0,	// Direct
	ALGORITHM_CONCOR_FREQ_DOMAIN = 1	// Frequency Domain
} AnalysisLibConvolveCorrelateAlgorithm;

/* Normalization used in NormalizedCorrelate */
typedef enum {
	ALGORITHM_CORCOR_NO_NORMALIZATION       = 0,	// No normalization
	ALGORITHM_CONCOR_UNBIASED_NORMALIZATION = 1,	// Unbiased normalization
	ALGORITHM_CONCOR_BIASED_NORMALIZATION   = 2	// Biased normalization
} AnalysisLibConvolveCorrelateNormalization;

/* The differentiation method used in DifferenceEx */
typedef enum {
	DIFF_SECOND_ORDER_CENTRAL = 0,		// Second order central
	DIFF_FOURTH_ORDER_CENTRAL = 1,		// Fourth order central
	DIFF_FORWARD              = 2,		// Forward
	DIFF_BACKWARD             = 3 		// Backward
} AnalysisLibDifferenceMethod;

/* The phase unit used in UnWrap1DByUnit */
typedef enum {
	PHASE_RADIAN_IN_RADIAN_OUT = 0,		// Radian in - radian out
	PHASE_RADIAN_IN_DEGREE_OUT = 1,		// Radian in - degree out
	PHASE_DEGREE_IN_DEGREE_OUT = 2,		// Degree in - degree out
	PHASE_DEGREE_IN_RADIAN_OUT = 3 		// Degree in - radian out
} AnalysisLibPhaseUnit;

/* The method for calculating StateLevels */
typedef enum {
	STATE_LEVELS_HISTOGRAM   = 0,		// Histogram
	STATE_LEVELS_PEAK        = 1,		// Peak
	STATE_LEVELS_AUTO_SELECT = 2 		// Auto select
} AnalysisLibStateLevelsMethod;

/* The export mode for tone extraction */
typedef enum {
  TONE_EXPORT_NONE        = 0,    // Export nothing
  TONE_EXPORT_INPUT       = 1,    // Export the input signal
  TONE_EXPORT_DETECTED    = 2,    // Export the detected signal
  TONE_EXPORT_RESIDUAL    = 3     // Export the residual signal
} AnalysisLibExportMode;

/* The output size for 2D Convolution */
typedef enum {
  OUTPUT_SIZE_FULL        = 0,		// Size of X and Y
  OUTPUT_SIZE_SIZE_X      = 1,		// Size of X
  OUTPUT_SIZE_COMPACT     = 2		// Compact
} AnalysisLibOutputSize;

/* The norm type used by UnitVector */
typedef enum {
  NORM_TYPE_1_            = 0,		// 1
  NORM_TYPE_2_            = 1,		// 2
  NORM_TYPE_INF_          = 2,		// +Infinity
  NORM_TYPE_NINF_         = 3,		// -Infinity
  NORM_TYPE_USER_         = 4		// User-defined
} AnalysisLibNormType_;

/* The autocorrelation method used by AutoCorrMtrx */
typedef enum {
  AUTO_CORR_AUTOCORRELATION       = 0,		// Autocorrelation
  AUTO_CORR_PRE_WINDOWED          = 1,		// Pre-windowed autocorrelation
  AUTO_CORR_POST_WINDOWED         = 2,		// Post-windowed autocorrelation
  AUTO_CORR_COVARIANCE_MATRIX     = 3,		// Covariance
  AUTO_CORR_MODIFIED_COVARIANCE   = 4		// Modified covariance
} AnalysisLibAutoCorrMethod;

/* Slope trigger used by TriggerDetection */
typedef enum {
  SLOPE_FALLING_EDGE   = 0,		// Detect falling edge
  SLOPE_RISING_EDGE    = 1		// Detect rising edge
} AnalysisLibTriggerSlope;

/*- Typedefs -----------------------------------------------------------------*/

/* Window constants used by measurement functions */
typedef struct {
    double enbw;
    double coherentgain;
} WindowConst;

/* Model function used by NonLinearFit() */
typedef double CVICALLBACK ModelFun(double x, double *coef, int ncoef);

/* Filter information used by IIR filter functions */
typedef struct {
    int type;       /* filter type (LOWPASS, etc.) */
    int order;      /* filter order */
    int reset;
    int na;
    double *a;      /* a coefficients */
    int nb;
    double *b;      /* b coefficients */
    int ns;
    double *s;      /* internal state */
} *IIRFilterPtr;

typedef struct {
	double amplitude;
	double frequencyLow;
	double frequencyHigh;
	double weightedRipple;
} BandParameter;

typedef struct {
	int fltType;
	int interp;
	int Mtaps;
	double *Mcoef;
	int Itaps;
	double *Icoef;
} FIRCoefStruct, *FIRCoefPtr;

typedef struct {
    double centerFrequency;
    double frequencyWidth;
} SearchType;

/* Define datatype NIComplexNumber */
#if !defined(_NIComplexNumber)
typedef struct NIComplexNumber_struct {
    double real;
    double imaginary;
} NIComplexNumber;
#define _NIComplexNumber
#endif
#define ComplexNum NIComplexNumber

/* IIR Filter State */
typedef struct {
	int numStates;     
	NIComplexNumber *states;
} *CxIIRFilterStatePtr;

/* FFT table used by FFT, spectrum related functions */
typedef void* PFFTTable;


/*- Signal Generation --------------------------------------------------------*/

AnalysisLibErrType CVIFUNC Impulse      (ssize_t n, double amp, ssize_t index, double x[]);
AnalysisLibErrType CVIFUNC Pulse        (ssize_t n, double amp, ssize_t delay,
                                         ssize_t width, double pulsePattern[]);
AnalysisLibErrType CVIFUNC Ramp         (ssize_t n, double first, double last,
                                         double rampvals[]);
AnalysisLibErrType CVIFUNC Triangle     (ssize_t n, double amp, double tri[]);
AnalysisLibErrType CVIFUNC SinePattern  (ssize_t n, double amp, double phase,
                                         double cycles, double sine[]);
AnalysisLibErrType CVIFUNC Uniform      (ssize_t n, int seed, double x[]);
AnalysisLibErrType CVIFUNC WhiteNoise   (ssize_t n, double amp, int seed,
                                         double noise[]);
AnalysisLibErrType CVIFUNC GaussNoise   (ssize_t n, double sDev, int seed,
                                         double noise[]);
AnalysisLibErrType CVIFUNC PeriodNoise  (ssize_t n, double amp, int seed,
										 double noise[]);
AnalysisLibErrType CVIFUNC GammaNoise   (ssize_t n, int order, int seed,
										 int initialize, double noise[]);
AnalysisLibErrType CVIFUNC PoissonNoise (ssize_t n, double mean, int seed,
										 int initialize, double noise[]);
AnalysisLibErrType CVIFUNC BinomialNoise(ssize_t n, int trials, double trialprob,
										 int seed, int initialize, int noise[]);
AnalysisLibErrType CVIFUNC BernoulliNoise(ssize_t n, double oneprob, int seed,
										  int initialize, int noise[]);
AnalysisLibErrType CVIFUNC ArbitraryWave(ssize_t n, double amp, double f,
                                         double *phase, const double wavetable[],
                                         ssize_t wavesize, int wavetype, double x[]);
AnalysisLibErrType CVIFUNC Chirp        (ssize_t n, double amp, double f1,
                                         double f2, double x[]);
AnalysisLibErrType CVIFUNC SawtoothWave (ssize_t n, double amp, double f,
                                         double *phase, double x[]);
AnalysisLibErrType CVIFUNC Sinc         (ssize_t n, double amp, double delay,
                                         double dt, double x[]);
AnalysisLibErrType CVIFUNC SineWave     (ssize_t n, double amp, double f,
                                         double *phase, double x[]);
AnalysisLibErrType CVIFUNC SquareWave   (ssize_t n, double amp, double f,
                                         double *phase, double duty, double x[]);
AnalysisLibErrType CVIFUNC TriangleWave (ssize_t n, double amp, double f,
                                         double *phase, double x[]);
AnalysisLibErrType CVIFUNC GaussMonopulse (ssize_t n, double amp, double delay,
					   double dt, double fc, double x[]);
AnalysisLibErrType CVIFUNC GaussModSinePattern (ssize_t n, double amp,
						double delay, double dt,
						double fc, double bandwidth,
						double attenuation, double x[]);
AnalysisLibErrType CVIFUNC PeriodicSinc (ssize_t n, double amp, double delay,
					 double dt, int c, double x[]);
AnalysisLibErrType CVIFUNC UnsymmetricTriangle (ssize_t n, double amp,
						double delay, double dt,
						double width, double asy,
						double x[]);
AnalysisLibErrType CVIFUNC RiffleArray   (double x[], ssize_t n, int seed,
									      ssize_t index[]);
AnalysisLibErrType CVIFUNC CxRiffleArray (NIComplexNumber x[], ssize_t n, int seed,
										  ssize_t index[]);
AnalysisLibErrType CVIFUNC IntRiffleArray(int x[], ssize_t n, int seed,
										  ssize_t index[]);
AnalysisLibErrType CVIFUNC HaltonSeq    (ssize_t n, int seed, int initialize,
										 double seq[]);
AnalysisLibErrType CVIFUNC RichtmeyerSeq(ssize_t n, double seed, int initialize,
										 double seq[]);



/*- 1-D Array Operations -----------------------------------------------------*/

AnalysisLibErrType CVIFUNC Clear1D  (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC Set1D    (double x[], ssize_t n, double a);
AnalysisLibErrType CVIFUNC Copy1D   (const double x[], ssize_t n, double y[]);
AnalysisLibErrType CVIFUNC Add1D    (const double x[], const double y[], ssize_t n,
                                     double z[]);
AnalysisLibErrType CVIFUNC Sub1D    (const double x[], const double y[], ssize_t n,
                                     double z[]);
AnalysisLibErrType CVIFUNC Mul1D    (const double x[], const double y[], ssize_t n,
                                     double z[]);
AnalysisLibErrType CVIFUNC Div1D    (const double x[], const double y[], ssize_t n,
                                     double z[]);
AnalysisLibErrType CVIFUNC Abs1D    (const double x[], ssize_t n, double y[]);
AnalysisLibErrType CVIFUNC Neg1D    (const double x[], ssize_t n, double y[]);
AnalysisLibErrType CVIFUNC LinEv1D  (const double x[], ssize_t n, double a,
                                     double b, double y[]);
AnalysisLibErrType CVIFUNC PolyEv1D (const double x[], ssize_t n,
                                     const double coef[], int k, double y[]);
AnalysisLibErrType CVIFUNC Scale1D  (const double x[], ssize_t n, double y[],
                                     double *offset, double *scale);
AnalysisLibErrType CVIFUNC QScale1D (const double x[], ssize_t n, double y[],
                                     double *scale);
AnalysisLibErrType CVIFUNC MaxMin1D (const double x[], ssize_t n, double *max,
                                     ssize_t *imax, double *min, ssize_t *imin);
AnalysisLibErrType CVIFUNC Sum1D    (const double x[], ssize_t n, double *sum);
AnalysisLibErrType CVIFUNC Prod1D   (const double x[], ssize_t n, double *prod);
AnalysisLibErrType CVIFUNC Subset1D (const double x[], ssize_t n, ssize_t index,
                                     ssize_t length, double y[]);
AnalysisLibErrType CVIFUNC Normal1D (const double x[], ssize_t n, double y[],
                                     double *ave, double *sDev);
AnalysisLibErrType CVIFUNC Reverse  (const double x[], ssize_t n, double y[]);
AnalysisLibErrType CVIFUNC Shift    (const double x[], ssize_t n, ssize_t shifts,
                                     double y[]);
AnalysisLibErrType CVIFUNC Clip     (const double x[], ssize_t n, double upper,
                                     double lower, double y[]);
AnalysisLibErrType CVIFUNC Sort     (const double x[], ssize_t n, int direction,
                                     double y[]);


/*- 2-D Array Operations -----------------------------------------------------*/

AnalysisLibErrType CVIFUNC Add2D    (const void *x, const void *y, ssize_t n, ssize_t m,
                                     void *z);
AnalysisLibErrType CVIFUNC Sub2D    (const void *x, const void *y, ssize_t n, ssize_t m,
                                     void *z);
AnalysisLibErrType CVIFUNC Mul2D    (const void *x, const void *y, ssize_t n, ssize_t m,
                                     void *z);
AnalysisLibErrType CVIFUNC Div2D    (const void *x, const void *y, ssize_t n, ssize_t m,
                                     void *z);
AnalysisLibErrType CVIFUNC LinEv2D  (const void *x, ssize_t n, ssize_t m, double a,
                                     double b, void *y);
AnalysisLibErrType CVIFUNC PolyEv2D (const void *x, ssize_t n, ssize_t m, double coef[],
                                     int k, void *y);
AnalysisLibErrType CVIFUNC Scale2D  (const void *x, ssize_t n, ssize_t m, void *y,
                                     double *offset, double *scale);
AnalysisLibErrType CVIFUNC QScale2D (const void *x, ssize_t n, ssize_t m, void *y,
                                     double *scale);
AnalysisLibErrType CVIFUNC MaxMin2D (const void *x, ssize_t n, ssize_t m, double *max,
                                     ssize_t *imax, ssize_t *jmax, double *min,
                                     ssize_t *imin, ssize_t *jmin);
AnalysisLibErrType CVIFUNC Sum2D    (const void *x, ssize_t n, ssize_t m, double *sum);
AnalysisLibErrType CVIFUNC Normal2D (const void *x, ssize_t n, ssize_t m, void *y,
                                     double *ave, double *sDev);


/*- Complex Number Operations ------------------------------------------------*/

AnalysisLibErrType CVIFUNC CxAdd   (double xr, double xi, double yr, double yi,
                                    double *zr, double *zi);
AnalysisLibErrType CVIFUNC CxSub   (double xr, double xi, double yr, double yi,
                                    double *zr, double *zi);
AnalysisLibErrType CVIFUNC CxMul   (double xr, double xi, double yr, double yi,
                                    double *zr, double *zi);
AnalysisLibErrType CVIFUNC CxDiv   (double xr, double xi, double yr, double yi,
                                    double *zr, double *zi);
AnalysisLibErrType CVIFUNC CxRecip (double xr, double xi, double *yr, double *yi);
AnalysisLibErrType CVIFUNC CxSqrt  (double xr, double xi, double *yr, double *yi);
AnalysisLibErrType CVIFUNC CxLog   (double xr, double xi, double *yr, double *yi);
AnalysisLibErrType CVIFUNC CxLn    (double xr, double xi, double *yr, double *yi);
AnalysisLibErrType CVIFUNC CxPow   (double xr, double xi, double a, double *yr,
                                    double *yi);
AnalysisLibErrType CVIFUNC CxExp   (double xr, double xi, double *yr, double *yi);
AnalysisLibErrType CVIFUNC ToPolar (double x, double y, double *mag,
                                    double *phase);
AnalysisLibErrType CVIFUNC ToRect  (double mag, double phase, double *x,
                                    double *y);


/*- 1-D Complex Array Operations ---------------------------------------------*/

AnalysisLibErrType CVIFUNC CxAdd1D    (const double xr[], const double xi[],
                                       const double yr[], const double yi[],
                                       ssize_t n, double zr[], double zi[]);


AnalysisLibErrType CVIFUNC CxSub1D    (const double xr[], const double xi[],
                                       const double yr[], const double yi[],
                                       ssize_t n, double zr[], double zi[]);

AnalysisLibErrType CVIFUNC CxMul1D    (const double xr[], const double xi[],
                                       const double yr[], const double yi[],
                                       ssize_t n, double zr[], double zi[]);

AnalysisLibErrType CVIFUNC CxDiv1D    (const double xr[], const double xi[],
                                       const double yr[], const double yi[],
                                       ssize_t n, double zr[], double zi[]);

AnalysisLibErrType CVIFUNC CxLinEv1D  (const double xr[], const double xi[],
                                       ssize_t n, double ar, double ai, double br,
                                       double bi, double yr[], double yi[]);

AnalysisLibErrType CVIFUNC ToPolar1D  (const double x[], const double y[],
                                       ssize_t n, double mag[], double phase[]);

AnalysisLibErrType CVIFUNC ToRect1D   (const double mag[], const double phase[],
                                       ssize_t n, double x[], double y[]);


/*- Frequency Domain Analysis ------------------------------------------------*/

AnalysisLibErrType CVIFUNC FFT      (double x[], double y[], ssize_t n);
AnalysisLibErrType CVIFUNC InvFFT   (double x[], double y[], ssize_t n);
AnalysisLibErrType CVIFUNC ReFFT    (double x[], double y[], ssize_t n);
AnalysisLibErrType CVIFUNC ReInvFFT (double x[], double y[], ssize_t n);
AnalysisLibErrType CVIFUNC Spectrum   (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxSpectrum (const NIComplexNumber x[], ssize_t n,
									   double spectrum[]);
AnalysisLibErrType CVIFUNC FHT      (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC InvFHT   (double x[], ssize_t n);

AnalysisLibErrType CVIFUNC CrossSpectrum  (const double x[], const double y[],
                                           ssize_t n, double zr[], double zi[]);
AnalysisLibErrType CVIFUNC CxCrossSpectrum(const NIComplexNumber x[],
										   const NIComplexNumber y[],
										   ssize_t n, NIComplexNumber z[]);
AnalysisLibErrType CVIFUNC FastHilbertTransform(double x[], ssize_t n);
AnalysisLibErrType CVIFUNC InvFastHilbertTransform(double x[], ssize_t n);

AnalysisLibErrType CVIFUNC FFT2D      (const void *a, ssize_t rowA, ssize_t colA,
                                       ssize_t fftSizeX, ssize_t fftSizeY, int shift, void *b);
AnalysisLibErrType CVIFUNC InvFFT2D   (const void *a, ssize_t rowA, ssize_t colA,
                                       int shifted, void *b);
AnalysisLibErrType CVIFUNC CxFFT2D    (const void *a, ssize_t rowA, ssize_t colA,
                                       ssize_t fftSizeX,ssize_t fftSizeY, int shift, void *b);
AnalysisLibErrType CVIFUNC CxInvFFT2D (const void *a, ssize_t rowA, ssize_t colA,
                                       int shifted, void *b);

AnalysisLibErrType CVIFUNC DCT        (const double x[], ssize_t n, ssize_t DCTSize, double y[]);
AnalysisLibErrType CVIFUNC InvDCT     (const double y[], ssize_t n,  double x[]);
AnalysisLibErrType CVIFUNC DST        (const double x[], ssize_t n, ssize_t DSTSize, double y[]);
AnalysisLibErrType CVIFUNC InvDST     (const double y[], ssize_t n,  double x[]);
AnalysisLibErrType CVIFUNC DCT2D      (const void *x, ssize_t rows, ssize_t cols, void *y);
AnalysisLibErrType CVIFUNC InvDCT2D   (const void *y, ssize_t rows, ssize_t cols, void *x);
AnalysisLibErrType CVIFUNC DST2D      (const void *x, ssize_t rows, ssize_t cols, void *y);
AnalysisLibErrType CVIFUNC InvDST2D   (const void *y, ssize_t rows, ssize_t cols, void *x);
AnalysisLibErrType CVIFUNC ChirpZT    (const double x[], ssize_t n,
                                       NIComplexNumber startingPoint,
                                       NIComplexNumber increment, int algorithm,
                                       ssize_t m, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxChirpZT  (const NIComplexNumber x[], ssize_t n,
                                       NIComplexNumber startingPoint,
                                       NIComplexNumber increment, int algorithm,
                                       ssize_t m, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC InvChirpZT (const NIComplexNumber x[], ssize_t m,
				       NIComplexNumber startPoint,
				       NIComplexNumber increment,
				       NIComplexNumber y[], ssize_t n);


/*- FFT Tables ----------------*/

PFFTTable CVIFUNC CreateFFTTable  (ssize_t n);
void      CVIFUNC DestroyFFTTable (PFFTTable table);

AnalysisLibErrType CVIFUNC FFTEx      (const double tmSig[], ssize_t nTmSig,
                                       ssize_t fftSize, PFFTTable fftTable,
                                       int shift, NIComplexNumber fft[]);
AnalysisLibErrType CVIFUNC InvFFTEx   (const NIComplexNumber fft[], ssize_t fftSize,
                                       PFFTTable fftTable, int shifted,
                                       double tmSig[]);
AnalysisLibErrType CVIFUNC CxFFTEx    (const NIComplexNumber tmSig[], ssize_t nTmSig,
                                       ssize_t fftSize, PFFTTable fftTable,
                                       int shift, NIComplexNumber fft[]);
AnalysisLibErrType CVIFUNC CxInvFFTEx (const NIComplexNumber fft[], ssize_t fftSize,
                                       PFFTTable fftTable, int shifted,
                                       NIComplexNumber tmSig[]);


/*- Time Domain Analysis -----------------------------------------------------*/

AnalysisLibErrType CVIFUNC Convolve   (const double x[], ssize_t n, const double y[],
                                       ssize_t m, double cxy[]);
AnalysisLibErrType CVIFUNC ConvolveEx (const double x[], ssize_t n, const double y[],
                                       ssize_t m, int algorithm, double cxy[]);
AnalysisLibErrType CVIFUNC Convolve2D(void *x, ssize_t m1, ssize_t n1,
	  								  void *y, ssize_t m2, ssize_t n2,
									  int outputSize, int algorithm, void *Cxy);
AnalysisLibErrType CVIFUNC CxConvolve(NIComplexNumber x[], ssize_t n,
									  NIComplexNumber y[], ssize_t m,
									  int algorithm, NIComplexNumber Cxy[]);
AnalysisLibErrType CVIFUNC CxConvolve2D(void *x, ssize_t m1, ssize_t n1,
										void *y, ssize_t m2, ssize_t n2,
										int outputSize, int algorithm, void *Cxy);
AnalysisLibErrType CVIFUNC Correlate  (const double x[], ssize_t n, const double y[],
                                       ssize_t m, double rxy[]);
AnalysisLibErrType CVIFUNC CorrelateEx (const double x[], ssize_t n, const double y[],
                                        ssize_t m, int algorithm, double rxy[]);
AnalysisLibErrType CVIFUNC Correlate2D(void *x, ssize_t m1, ssize_t n1,
									   void *y, ssize_t m2, ssize_t n2,
									   int algorithm, void *Rxy);
AnalysisLibErrType CVIFUNC CxCorrelate(NIComplexNumber x[], ssize_t n,
									   NIComplexNumber y[], ssize_t m,
									   int algorithm, NIComplexNumber Rxy[]);
AnalysisLibErrType CVIFUNC CxCorrelate2D(void *x, ssize_t m1, ssize_t n1,
										 void *y, ssize_t m2, ssize_t n2,
										 int algorithm, void *Rxy);
AnalysisLibErrType CVIFUNC NormalizedCorrelate (const double x[], ssize_t n, const double y[],
                                                ssize_t m, int algorithm, int normalization, double rxy[]);
AnalysisLibErrType CVIFUNC CxNormalizedCorrelate(NIComplexNumber x[], ssize_t n,
												 NIComplexNumber y[], ssize_t m,
												 int algorithm, int normalization, NIComplexNumber Rxy[]);
AnalysisLibErrType CVIFUNC AutoCorrelate(double x[], ssize_t n,
										 int normalization, double Rxx[]);
AnalysisLibErrType CVIFUNC AutoCorrelate2D(void *x, ssize_t m, ssize_t n,
										   void *Rxx);
AnalysisLibErrType CVIFUNC CxAutoCorrelate(NIComplexNumber x[], ssize_t n,
										   int norm, NIComplexNumber Rxx[]);
AnalysisLibErrType CVIFUNC CxAutoCorrelate2D(void *x, ssize_t m, ssize_t n,
											 void *Rxx);
AnalysisLibErrType CVIFUNC Integrate  (const double x[], ssize_t n, double dt,
                                       double xInit, double xFinal, double y[]);
AnalysisLibErrType CVIFUNC Difference (const double x[], ssize_t n, double dt,
                                       double xInit, double xFinal, double y[]);
AnalysisLibErrType CVIFUNC DifferenceEx (double x[], ssize_t n, double dt,
                                         const double xInit[], ssize_t xInitSize,
                                         const double xFinal[],	ssize_t xFinalSize,
                                         int differenceMethod, double y[]);
AnalysisLibErrType CVIFUNC PulseParam (const double pulsePattern[], ssize_t n,
                                       double *amp, double *amp90,
                                       double *amp50, double *amp10,
                                       double *top, double *base,
                                       double *topOvershoot,
                                       double *baseOvershoot, ssize_t *delay,
                                       ssize_t *width, ssize_t *riseTime, ssize_t *fallTime,
                                       double *slewRate);
AnalysisLibErrType CVIFUNC Decimate   (const double x[], ssize_t n, ssize_t D, int avg,
                                       double y[]);
AnalysisLibErrType CVIFUNC DecimateContinuous(double x[], ssize_t nx, ssize_t D, int avg,
											  ssize_t startIndex, int initialize, ssize_t *ny);
AnalysisLibErrType CVIFUNC CxDecimate (const NIComplexNumber x[], ssize_t n, ssize_t D,
				                       int avg, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxDecimateContinuous(NIComplexNumber x[], ssize_t nx, ssize_t D, int avg,
												ssize_t startIndex, int initialize, ssize_t *ny);
AnalysisLibErrType CVIFUNC Deconvolve (const double y[], ssize_t ny,
                                       const double x[], ssize_t nx, double h[]);
AnalysisLibErrType CVIFUNC UnWrap1D   (double phase[], ssize_t n);
AnalysisLibErrType CVIFUNC UnWrap1DByUnit (double phase[], ssize_t n, int unit);
AnalysisLibErrType CVIFUNC TriggerDetection(double x[], ssize_t n, double threshold,
	  										double hysteresis, int slope, int initialize,
											ssize_t *index, int *triggered);
AnalysisLibErrType CVIFUNC TriggerDetection2D(void *x, ssize_t m, ssize_t n,
											  double threshold[], double hysteresis[],
											  int slope[], int initialize,
											  ssize_t *index, int *triggered);



/*- IIR Digital Filters ------------------------------------------------------*/

AnalysisLibErrType CVIFUNC Bessel_CascadeCoef(double fs, double fl, double fh,
                                              IIRFilterPtr filterInfo);

AnalysisLibErrType CVIFUNC Bessel_CascadeCoefEx(double fs, double fl, double fh,
												IIRFilterPtr filterInfo);

AnalysisLibErrType CVIFUNC Bw_CascadeCoef    (double fs, double fl, double fh,
                                              IIRFilterPtr filterInfo);

AnalysisLibErrType CVIFUNC Ch_CascadeCoef    (double fs, double fl, double fh,
                                              double r, IIRFilterPtr filterInfo);

AnalysisLibErrType CVIFUNC InvCh_CascadeCoef (double fs, double fl, double fh,
                                              double atten, IIRFilterPtr filterInfo);

AnalysisLibErrType CVIFUNC Elp_CascadeCoef   (double fs, double fl, double fh,
                                              double ripple, double atten,
                                              IIRFilterPtr filterInfo);

AnalysisLibErrType CVIFUNC IIRCascadeFiltering(const double x[], ssize_t n,
                                               IIRFilterPtr filterInfo, double y[]);

AnalysisLibErrType CVIFUNC CxIIRCascadeFiltering(const NIComplexNumber x[], ssize_t n,
												 IIRFilterPtr filterInfo,
												 CxIIRFilterStatePtr filterState,
												 NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC ZeroPhaseFiltering(const double x[], ssize_t n,
                                              const double a[], int na,
                                              const double b[], int nb, double y[]);

AnalysisLibErrType CVIFUNC MedianFilter(double x[], ssize_t n,
										ssize_t leftRank, ssize_t rightRank, double y[]);

IIRFilterPtr       CVIFUNC AllocIIRFilterPtr(int type, int order);

void               CVIFUNC ResetIIRFilter   (IIRFilterPtr filterInfo);

void               CVIFUNC FreeIIRFilterPtr (IIRFilterPtr filterInfo);

CxIIRFilterStatePtr CVIFUNC AllocCxIIRFilterStatePtr(int type, int order);

void CVIFUNC FreeCxIIRFilterStatePtr(CxIIRFilterStatePtr filterState);

AnalysisLibErrType CVIFUNC CascadeToDirectCoef(IIRFilterPtr filterInfo, double a[],
                                               int na, double b[], int nb);


AnalysisLibErrType CVIFUNC Bssl_LPF   (double x[], ssize_t n, double fs,
									   double fc, int order, double y[]);

AnalysisLibErrType CVIFUNC Bssl_HPF   (double x[], ssize_t n, double fs,
									   double fc, int order, double y[]);

AnalysisLibErrType CVIFUNC Bssl_BPF   (double x[], ssize_t n, double fs,
									   double fl, double fh, int order,
									   double y[]);

AnalysisLibErrType CVIFUNC Bssl_BSF   (double x[], ssize_t n, double fs,
									   double fl, double fh, int order,
									   double y[]);

AnalysisLibErrType CVIFUNC Bw_LPF     (const double x[], ssize_t n, double fs,
                                       double fc, int order, double y[]);

AnalysisLibErrType CVIFUNC Bw_HPF     (const double x[], ssize_t n, double fs,
                                       double fc, int order, double y[]);

AnalysisLibErrType CVIFUNC Bw_BPF     (const double x[], ssize_t n, double fs,
                                       double fl, double fh, int order,
                                       double y[]);

AnalysisLibErrType CVIFUNC Bw_BSF     (const double x[], ssize_t n, double fs,
                                       double fl, double fh, int order,
                                       double y[]);

AnalysisLibErrType CVIFUNC Ch_LPF     (const double x[], ssize_t n, double fs,
                                       double fc, double ripple,
                                       int order, double y[]);

AnalysisLibErrType CVIFUNC Ch_HPF     (const double x[], ssize_t n, double fs,
                                       double fc, double ripple,
                                       int order, double y[]);

AnalysisLibErrType CVIFUNC Ch_BPF     (const double x[], ssize_t n, double fs,
                                       double fl, double fh,
                                       double ripple, int order, double y[]);

AnalysisLibErrType CVIFUNC Ch_BSF     (const double x[], ssize_t n, double fs,
                                       double fl, double fh,
                                       double ripple, int order, double y[]);

AnalysisLibErrType CVIFUNC InvCh_LPF  (const double x[], ssize_t n, double fs,
                                       double fc, double atten, int order,
                                       double y[]);

AnalysisLibErrType CVIFUNC InvCh_HPF  (const double x[], ssize_t n, double fs,
                                       double fc, double atten, int order,
                                       double y[]);

AnalysisLibErrType CVIFUNC InvCh_BPF  (const double x[], ssize_t n, double fs,
                                       double fl, double fh, double atten,
                                       int order, double y[]);

AnalysisLibErrType CVIFUNC InvCh_BSF  (const double x[], ssize_t n, double fs,
                                       double fl, double fh, double atten,
                                       int order, double y[]);

AnalysisLibErrType CVIFUNC Elp_LPF    (const double x[], ssize_t n, double fs,
                                       double fc, double ripple, double atten,
                                       int order, double y[]);

AnalysisLibErrType CVIFUNC Elp_HPF    (const double x[], ssize_t n, double fs,
                                       double fc, double ripple, double atten,
                                       int order, double y[]);

AnalysisLibErrType CVIFUNC Elp_BPF    (const double x[], ssize_t n, double fs,
                                       double fl, double fh, double ripple,
                                       double atten, int order, double y[]);

AnalysisLibErrType CVIFUNC Elp_BSF    (const double x[], ssize_t n, double fs,
                                       double fl, double fh, double ripple,
                                       double atten, int order, double y[]);


AnalysisLibErrType CVIFUNC CxBssl_LPF (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxBssl_HPF (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxBssl_BPF (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxBssl_BSF (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxBw_LPF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxBw_HPF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxBw_BPF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh, int order,
									   NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxBw_BSF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh, int order,
									   NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxCh_LPF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, double ripple, int order,
									   NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxCh_HPF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, double ripple, int order,
									   NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxCh_BPF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh,
									   double ripple, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxCh_BSF   (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh,
									   double ripple, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxInvCh_LPF(NIComplexNumber x[], ssize_t n, double fs,
									   double fc, double atten, int order,
									   NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxInvCh_HPF(NIComplexNumber x[], ssize_t n, double fs,
									   double fc, double atten, int order,
									   NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxInvCh_BPF(NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh,
									   double atten, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxInvCh_BSF(NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh,
									   double atten, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxElp_LPF  (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, double ripple, double atten,
									   int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxElp_HPF  (NIComplexNumber x[], ssize_t n, double fs,
									   double fc, double ripple, double atten,
									   int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxElp_BPF  (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh, double ripple,
									   double atten, int order, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC CxElp_BSF  (NIComplexNumber x[], ssize_t n, double fs,
									   double fl, double fh, double ripple,
									   double atten, int order, NIComplexNumber y[]);


/*- Old-style IIR Filter Functions -*/

AnalysisLibErrType CVIFUNC Bessel_Coef(int type, int order, double fs,
                                       double fl, double fh, double a[],
                                       int na, double b[], int nb);

AnalysisLibErrType CVIFUNC Bw_Coef    (int type, int order, double fs,
                                       double fl, double fh, double a[],
                                       int na, double b[], int nb);

AnalysisLibErrType CVIFUNC Ch_Coef    (int type, int order, double fs,
                                       double fl, double fh, double r,
                                       double a[], int na, double b[], int nb);

AnalysisLibErrType CVIFUNC InvCh_Coef (int type, int order, double fs,
                                       double fl, double fh, double fr,
                                       double a[], int na, double b[], int nb);

AnalysisLibErrType CVIFUNC Elp_Coef   (int type, int order, double fs,
                                       double fl, double fh, double r1,
                                       double r2, double a[], int na,
                                       double b[], int nb);

AnalysisLibErrType CVIFUNC IIRFiltering(const double x[], ssize_t n,
                                        const double a[], double y1[], int na,
                                        const double b[], double x1[], int nb,
                                        double y[]);
AnalysisLibErrType CVIFUNC IIRFiltering_CxInput(const NIComplexNumber x[],
						ssize_t n, const double a[],
						NIComplexNumber y1[],
						int na,	const double b[],
						NIComplexNumber x1[],
						int nb, NIComplexNumber y[]);


/*- FIR Digital Filters ------------------------------------------------------*/

AnalysisLibErrType CVIFUNC Wind_LPF    (double fs, double fc, int n,
                                        double coef[], int windType);
AnalysisLibErrType CVIFUNC Wind_HPF    (double fs, double fc, int n,
                                        double coef[], int windType);
AnalysisLibErrType CVIFUNC Wind_BPF    (double fs, double fl, double fh, int n,
                                        double coef[], int windType);
AnalysisLibErrType CVIFUNC Wind_BSF    (double fs, double fl, double fh, int n,
                                        double coef[], int windType);
AnalysisLibErrType CVIFUNC Ksr_LPF     (double fs, double fc, int n,
                                        double coef[], double beta);
AnalysisLibErrType CVIFUNC Ksr_HPF     (double fs, double fc, int n,
                                        double coef[], double beta);
AnalysisLibErrType CVIFUNC Ksr_BPF     (double fs, double fl, double fh, int n,
                                        double coef[], double beta);
AnalysisLibErrType CVIFUNC Ksr_BSF     (double fs, double fl, double fh, int n,
                                        double coef[], double beta);
AnalysisLibErrType CVIFUNC Equi_Ripple (int bands, const double A[],
                                        const double wts[], double fs,
                                        const double cutoffs[], int type, int n,
                                        double coef[], double *delta);
AnalysisLibErrType CVIFUNC EquiRpl_LPF (double fs, double f1, double f2, int n,
                                        double coef[], double *delta);
AnalysisLibErrType CVIFUNC EquiRpl_HPF (double fs, double f1, double f2, int n,
                                        double coef[], double *delta);
AnalysisLibErrType CVIFUNC EquiRpl_BPF (double fs, double f1, double f2,
                                        double f3, double f4, int n,
                                        double coef[], double *delta);
AnalysisLibErrType CVIFUNC EquiRpl_BSF (double fs, double f1, double f2,
                                        double f3, double f4, int n,
                                        double coef[], double *delta);

AnalysisLibErrType CVIFUNC EquiRpl_LPFiltering(double x[], ssize_t n, int ncoef,
											   double fs, double fh, double fl,
											   double y[]);
AnalysisLibErrType CVIFUNC EquiRpl_HPFiltering(double x[], ssize_t n, int ncoef,
											   double fs, double f1, double f2,
											   double y[]);
AnalysisLibErrType CVIFUNC EquiRpl_BPFiltering(double x[], ssize_t n, int ncoef,
											   double fs, double f1, double f2,
											   double f3, double f4, double y[]);
AnalysisLibErrType CVIFUNC EquiRpl_BSFiltering(double x[], ssize_t n, int ncoef,
											   double fs, double f1, double f2,
											   double f3, double f4, double y[]);
AnalysisLibErrType CVIFUNC CxEquiRpl_LPFiltering(NIComplexNumber x[], ssize_t n,
												 int ncoef, double fs, double f1,
												 double f2, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxEquiRpl_HPFiltering(NIComplexNumber x[], ssize_t n,
												 int ncoef, double fs, double f1,
												 double f2, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxEquiRpl_BPFiltering(NIComplexNumber x[], ssize_t n,
												 int ncoef, double fs, double f1,
												 double f2, double f3, double f4,
												 NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxEquiRpl_BSFiltering(NIComplexNumber x[], ssize_t n,
												 int ncoef, double fs, double f1,
												 double f2, double f3, double f4,
												 NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC Wind_LPFiltering(double x[], ssize_t n, int ncoef,
											double fs, double fc, int winType,
											double winPara, double y[]);
AnalysisLibErrType CVIFUNC Wind_HPFiltering(double x[], ssize_t n, int ncoef,
											double fs, double fc, int winType,
											double winPara, double y[]);
AnalysisLibErrType CVIFUNC Wind_BPFiltering(double x[], ssize_t n, int ncoef,
											double fs, double fl, double fh,
											int winType, double winPara, double y[]);
AnalysisLibErrType CVIFUNC Wind_BSFiltering(double x[], ssize_t n, int ncoef,
											double fs, double fl, double fh,
											int winType, double winPara, double y[]);

AnalysisLibErrType CVIFUNC CxWind_LPFiltering(NIComplexNumber x[], ssize_t n,
											  int ncoef, double fs, double fc,
											  int winType, double winPara,
											  NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxWind_HPFiltering(NIComplexNumber x[], ssize_t n,
											  int ncoef, double fs, double fc,
											  int winType, double winPara,
											  NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxWind_BPFiltering(NIComplexNumber x[], ssize_t n,
											  int ncoef, double fs, double fl,
											  double fh, int winType,
											  double winPara, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxWind_BSFiltering(NIComplexNumber x[], ssize_t n,
											  int ncoef, double fs, double fl,
											  double fh, int winType,
											  double winPara, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC FIR_Coef (int type, double fs, double fl,
                                     double fh, int taps, double *coef);
AnalysisLibErrType CVIFUNC FIRFiltering (const double x[], ssize_t n,
					 const double b[], double x1[],
					 int nb, double y[]);
AnalysisLibErrType CVIFUNC FIRFiltering_CxInput (const NIComplexNumber x[],
						 ssize_t n, const double b[],
						 NIComplexNumber x1[], int nb,
						 NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC FIRNarrowBandCoef(double fs, double fbp, double fbs, double fc,
											 double ripple, double atten, int filterType,
											 FIRCoefStruct *coefinfo);
AnalysisLibErrType CVIFUNC FIRNarrowBandFilter(double x[], ssize_t n,
											   FIRCoefStruct coefinfo, double y[]);
AnalysisLibErrType CVIFUNC CxFIRNarrowBandFilter(NIComplexNumber x[], ssize_t n,
												 FIRCoefStruct coefinfo, NIComplexNumber y[]);

FIRCoefPtr CVIFUNC AllocFIRFilterPtr(void);
void CVIFUNC FreeFIRFilterPtr(FIRCoefPtr filterInfo);
 
AnalysisLibErrType CVIFUNC WindFIR_Filter (int filterType, double fs, double fl,
										   double fh, int taps, int winType,
										   double winPara, double coef[]);
AnalysisLibErrType CVIFUNC WindFIR_Filtering(double x[], ssize_t n, int ncoef,
											 int filterType, double fs,
											 double fl, double fh, int winType,
											 double winPara, double y[]);
AnalysisLibErrType CVIFUNC CxWindFIR_Filtering(NIComplexNumber x[], ssize_t n,
											   int ncoef, int filterType,
											   double fs, double fl, double fh,
											   int winType, double winPara,
											   NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC SavitzkyGolayCoef (int polyOrder, int nSidePoints,
					      double weight[], int needDiff,
					      double filterCoef[],
					      double diffCoef[]);
AnalysisLibErrType CVIFUNC SavitzkyGolayFiltering (double x[], ssize_t nx,
						   int polyOrder,
						   int nSidePoints,
						   double weight[], double y[]); 
AnalysisLibErrType CVIFUNC SavitzkyGolayFiltering_CxInput (NIComplexNumber x[],
							   ssize_t nx,
							   int polyOrder,
							   int nSidePoints,
							   double weight[],
							   NIComplexNumber y[]);
 
AnalysisLibErrType CVIFUNC Parks_McClellanCoef(int ncoefs, double fs, BandParameter bandparams[],
											   ssize_t bandnum, int type, double coef[],
											   double *ripple);


/*- Windows ------------------------------------------------------------------*/

AnalysisLibErrType CVIFUNC TriWin           (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC HanWin           (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC HamWin           (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC BkmanWin         (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC KsrWin           (double x[], ssize_t n, double beta);
AnalysisLibErrType CVIFUNC BlkHarrisWin     (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC CosTaperedWin    (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC CosTaperedWinEx  (double x[], ssize_t n, double r);
AnalysisLibErrType CVIFUNC ExBkmanWin       (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC ExpWin           (double x[], ssize_t n, double final);
AnalysisLibErrType CVIFUNC FlatTopWin       (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC ForceWin         (double x[], ssize_t n, double duty);
AnalysisLibErrType CVIFUNC GenCosWin        (double x[], ssize_t n, const double a[], int m);
AnalysisLibErrType CVIFUNC ChebWin          (double x[], ssize_t n, double s);
AnalysisLibErrType CVIFUNC GaussWin         (double x[], ssize_t n, double dev);
AnalysisLibErrType CVIFUNC SymWin           (double x[], ssize_t n, int winType, double para);
AnalysisLibErrType CVIFUNC BlkmanNuttallWin (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC ScaledWindowEx (double x[], ssize_t n, int winType,
                                              double para, WindowConst *windowconstants);
AnalysisLibErrType CVIFUNC ParzenWin (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC WelchWin (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC BartHannWin (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC BohmanWin (double x[], ssize_t n);
AnalysisLibErrType CVIFUNC GetWinProperties    (int winType, WindowConst *windowconstants);
AnalysisLibErrType CVIFUNC NumericWinProperties(const double coef[], int nCoef,
                                                WindowConst *windowconstants);
AnalysisLibErrType CVIFUNC NameWinProperties(ssize_t n, int winType, double para,
	  										 WindowConst *windowconstants);

/*- Complex Windows ----------------------------------------------------------*/

AnalysisLibErrType CVIFUNC CxTriWin          (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxHanWin          (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxHamWin          (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxBkmanWin        (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxKsrWin          (NIComplexNumber x[], ssize_t n, double beta);
AnalysisLibErrType CVIFUNC CxBlkHarrisWin    (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxCosTaperedWin   (NIComplexNumber x[], ssize_t n, double r);
AnalysisLibErrType CVIFUNC CxExBkmanWin      (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxExpWin          (NIComplexNumber x[], ssize_t n, double final);
AnalysisLibErrType CVIFUNC CxFlatTopWin      (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxForceWin        (NIComplexNumber x[], ssize_t n, double duty);
AnalysisLibErrType CVIFUNC CxGenCosWin       (NIComplexNumber x[], ssize_t n,
									          const double a[], int m);
AnalysisLibErrType CVIFUNC CxChebWin         (NIComplexNumber x[], ssize_t n, double s);
AnalysisLibErrType CVIFUNC CxGaussWin        (NIComplexNumber x[], ssize_t n, double dev);
AnalysisLibErrType CVIFUNC CxSymWin          (NIComplexNumber x[], ssize_t n, int winType,
											  double para);
AnalysisLibErrType CVIFUNC CxBlkmanNuttallWin(NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxScaledWindow    (NIComplexNumber x[], ssize_t n, int winType,
										      double para, WindowConst *windowconstants);
AnalysisLibErrType CVIFUNC CxParzenWin       (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxWelchWin        (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxBartHannWin     (NIComplexNumber x[], ssize_t n);
AnalysisLibErrType CVIFUNC CxBohmanWin       (NIComplexNumber x[], ssize_t n);


/*- Measurement --------------------------------------------------------------*/

AnalysisLibErrType CVIFUNC ACDCEstimator     (const double x[], ssize_t n,
                                              double *acestimate,
                                              double *dcestimate);
AnalysisLibErrType CVIFUNC AmpPhaseSpectrum  (const double x[], ssize_t n,
                                              int unwrap, double dt,
                                              double ampspectrum[],
                                              double phasespectrum[],
                                              double *df);
AnalysisLibErrType CVIFUNC AutoPowerSpectrum (const double x[], ssize_t n,
                                              double dt, double autospectrum[],
                                              double *df);
AnalysisLibErrType CVIFUNC CrossPowerSpectrum(const double x[], const double y[],
                                              ssize_t n, double dt, double magsxy[],
                                              double phasesxy[], double *df);
AnalysisLibErrType CVIFUNC ImpulseResponse   (const double stimulus[],
                                              const double response[], ssize_t n,
                                              double impulse[]);
AnalysisLibErrType CVIFUNC NetworkFunctions  (void *stimulus, void *response,
                                              ssize_t n, ssize_t numframes, double dt,
                                              double magsxy[],
                                              double phasesxy[],
                                              double magtransfer[],
                                              double phasetransfer[],
                                              double coherence[],
                                              double impulse[], double *df);
AnalysisLibErrType CVIFUNC PowerFrequencyEstimate (const double autospectrum[],
                                                   ssize_t n, double searchfreq,
                                                   WindowConst windowconstants,
                                                   double df, ssize_t span,
                                                   double *freqpeak,
                                                   double *powerpeak);
AnalysisLibErrType CVIFUNC ScaledWindow      (double x[], ssize_t n, int window,
                                              WindowConst *windowconstants);
AnalysisLibErrType CVIFUNC SpectrumUnitConversion (const double spectrum[],
                                                   ssize_t ns, int type, int loglin,
                                                   int unitselector, double df,
                                                   WindowConst windowconstants,
                                                   double convertedspectrum[],
                                                   char *unitstring);
AnalysisLibErrType CVIFUNC TransferFunction  (const double stimulus[],
                                              const double response[], ssize_t n,
                                              double dt, double magtransfer[],
                                              double phasetransfer[],
                                              double *df);
AnalysisLibErrType CVIFUNC HarmonicAnalyzer (const double x[], ssize_t n, ssize_t frame_size, int number_of_harmonics,
											 int window_type, double sampling_rate, double fundamental_frequency,
											 double harmonic_amplitudes[], double harmonic_frequencies[],
					    					 double *thd, double *thdnoise);
AnalysisLibErrType CVIFUNC HarmonicAnalyzerUsingSignal (const double x[], ssize_t n,
         int number_of_harmonics, double sampling_rate, double fundamental_frequency,
         double harmonic_amplitudes[], double harmonic_frequencies[],
         double *thd, double *thdnoise);
AnalysisLibErrType CVIFUNC StateLevels (const double x[], ssize_t n, int method, ssize_t histoSize,
         double *high, double *low, double *amp);
AnalysisLibErrType CVIFUNC PulseMeas (const double x[], ssize_t n, int polarity,
	ssize_t pulseNum, double highRef, double midRef, double lowRef, int refUnits,
	int method, ssize_t histoSize, double *period, double *pulseDuration,
	double *dutyCycle, double *pulseCenter, double *lowRefVal,
	double *midRefVal, double *highRefVal);
AnalysisLibErrType CVIFUNC TransMeas (double x[], ssize_t n, int polarity,
	ssize_t edgeNum, double highRef, double midRef, double lowRef, int refUnits,
	int method, ssize_t histoSize, double *slope, double *duration,
	double *startTime, double *endTime, double *lowRefVal, double *midRefVal,
	double *highRefVal, double *preUndershoot, double *preOvershoot,
	double *postUndershoot, double *postOvershoot);
AnalysisLibErrType CVIFUNC CycleRMSAverage (double x[], ssize_t n, ssize_t cycleNum,
	double highRef, double midRef, double lowRef, int refUnits, int method,
	ssize_t histoSize, double *cycleAvg, double *cycleRMS, double *startTime,
	double *endTime, double *lowRefVal, double *midRefVal, double *highRefVal);

AnalysisLibErrType CVIFUNC SingleToneInfo(const double x[], ssize_t n, double dt,
    SearchType *search, double *frequency, double *amplitude, double *phase);
	
AnalysisLibErrType CVIFUNC SingleToneSignal(const double x[], ssize_t n, double dt,
    SearchType *search, int exportMode, double *frequency, double *amplitude, double *phase,
    double exportedSignal[], double *exportedSamplePeriod,
    double exportedSpectrum[], double *startFrequency, double *frequencyResolution);	

AnalysisLibErrType CVIFUNC MultipleToneInfo(const double x[], ssize_t n, double dt,
    int sortOrder, double threshold, int maxTones, int *detectedTones,
    double frequency[], double amplitude[], double phase[]);
AnalysisLibErrType CVIFUNC MultipleToneSignal(const double x[], ssize_t n, double dt,
    int sortOrder, int exportMode, double threshold, int maxTones, int *detectedTones,
		double frequency[], double amplitude[], double phase[],
		double exportedSignal[], double *exportedSamplePeriod,
		double exportedSpectrum[], double *startFrequency, double *frequencyResolution);
	
AnalysisLibErrType CVIFUNC CxSingleToneInfo(const NIComplexNumber x[], ssize_t n, double dt,
    SearchType *search, double *frequency, double *amplitude, double *phase);
AnalysisLibErrType CVIFUNC CxSingleToneSignal(const NIComplexNumber x[], ssize_t n, double dt,
    SearchType *search, int exportMode,
    double *frequency, double *amplitude, double *phase,
    NIComplexNumber exportedSignal[], double *exportedSamplePeriod,
    double exportedSpectrum[], double *startFrequency, double *frequencyResolution);	
AnalysisLibErrType CVIFUNC CxMultipleToneInfo(const NIComplexNumber x[], ssize_t n, double dt,
    int sortOrder, double threshold, int maxTones, int *detectedTones,
    double frequency[], double amplitude[], double phase[]);
AnalysisLibErrType CVIFUNC CxMultipleToneSignal(const NIComplexNumber x[], ssize_t n, double dt,
    int sortOrder, int exportMode, double threshold, int maxTones, int *detectedTones,
    double frequency[], double amplitude[], double phase[],
    NIComplexNumber exportedSignal[], double *exportedSamplePeriod,
    double exportedSpectrum[], double *startFrequency, double *frequencyResolution);	

/*- General Statistics -------------------------------------------------------*/

AnalysisLibErrType CVIFUNC Mean      (const double x[], ssize_t n, double *mean);
AnalysisLibErrType CVIFUNC StdDev    (const double x[], ssize_t n, double *mean,
                                      double *sDev);
AnalysisLibErrType CVIFUNC Variance  (const double x[], ssize_t n, double *mean,
                                      double *var);
AnalysisLibErrType CVIFUNC RMS       (const double x[], ssize_t n, double *rms);
AnalysisLibErrType CVIFUNC CxRMS     (const NIComplexNumber x[], ssize_t n, double *rms);
AnalysisLibErrType CVIFUNC Moment    (const double x[], ssize_t n, int order,
                                      double *moment);
AnalysisLibErrType CVIFUNC Median    (const double x[], ssize_t n, double *median);
AnalysisLibErrType CVIFUNC Mode      (const double x[], ssize_t n, double base,
                                      double top, ssize_t intervals, double *mode);
AnalysisLibErrType CVIFUNC ModeEx    (const double x[], ssize_t n, ssize_t intervals,
                                      double *modes, ssize_t *m);
AnalysisLibErrType CVIFUNC Histogram (const double x[], ssize_t n, double base,
                                      double top, ssize_t hist[], double axis[],
									  ssize_t intervals);


/*- Probability Distribution -------------------------------------------------*/

AnalysisLibErrType CVIFUNC N_Dist     (double x, double *p);
AnalysisLibErrType CVIFUNC T_Dist     (double t, int n, double *p);
AnalysisLibErrType CVIFUNC F_Dist     (double f, int n, int m, double *p);
AnalysisLibErrType CVIFUNC XX_Dist    (double x, int n, double *p);
AnalysisLibErrType CVIFUNC InvN_Dist  (double p, double *x);
AnalysisLibErrType CVIFUNC InvT_Dist  (double p, int n, double *t);
AnalysisLibErrType CVIFUNC InvF_Dist  (double p, int n, int m, double *f);
AnalysisLibErrType CVIFUNC InvXX_Dist (double p, int n, double *x);


/*- Analysis of Variance -----------------------------------------------------*/

AnalysisLibErrType CVIFUNC ANOVA1Way (const double y[], const ssize_t level[],
                                      ssize_t n, ssize_t k, double *ssa, double *msa,
                                      double *f, double *sig, double *sse,
                                      double *mse, double *tss);
AnalysisLibErrType CVIFUNC ANOVA2Way (const double y[], const ssize_t levelA[],
                                      const ssize_t levelB[], ssize_t N, ssize_t L,
                                      ssize_t a, ssize_t b, double info[4][4],
                                      double *sigA, double *sigB, double *sigAB);
AnalysisLibErrType CVIFUNC ANOVA3Way (const double y[], const ssize_t levelA[],
                                      const ssize_t levelB[], const ssize_t levelC[],
                                      ssize_t N, ssize_t L, ssize_t a, ssize_t b, ssize_t c,
                                      double info[8][4], double *sigA,
                                      double *sigB, double *sigC, double *sigAB,
                                      double *sigAC, double *sigBC,
                                      double *sigABC);


/*- Nonparametric Statistics -------------------------------------------------*/

AnalysisLibErrType CVIFUNC Contingency_Table (int s, int k, const void *y,
                                              double *Test_Stat, double *Sig);

AnalysisLibErrType CVIFUNC Erf (double x, double *erfx);
AnalysisLibErrType CVIFUNC Erfc (double x, double *erfcx);


/*- Curve Fitting ------------------------------------------------------------*/

AnalysisLibErrType CVIFUNC LinFit       (const double x[], const double y[],
                                         ssize_t n, double z[], double *slope,
                                         double *intercept, double *mse);

AnalysisLibErrType CVIFUNC ExpFit       (const double x[], const double y[],
                                         ssize_t n, double z[], double *a,
                                         double *b, double *mse);

AnalysisLibErrType CVIFUNC PolyFit      (const double x[], const double y[],
                                         ssize_t n, int order, double z[],
                                         double coef[], double *mse);

AnalysisLibErrType CVIFUNC GenLSFit     (const void *H, ssize_t row, ssize_t col,
                                         const double y[], const double stddev[],
                                         int algorithm,
                                         double z[], double coef[],
                                         void *covar, double *mse);

AnalysisLibErrType CVIFUNC NonLinearFit (const double x[], const double y[],
                                         double z[], ssize_t n, ModelFun *func,
                                         double coef[], int m, double *mse);

AnalysisLibErrType CVIFUNC NonLinearFitWithMaxIters (const double x[], const double y[],
													const double z[], ssize_t n,
     												int maxiters, ModelFun *func,
     												double coef[], int m, double *mse);

AnalysisLibErrType CVIFUNC NonLinearFitWithWeight (const double x[], const double y[],
   													const double weight[], double z[], ssize_t n,
													int maxiters, ModelFun *func, 
													double coef[], int m, double *mse, int *iter);


AnalysisLibErrType CVIFUNC GaussFit      (const double x[], double y[], const double weight[],
										  ssize_t n, int fitMethod, double tolerance,
										  double initCoef[], double fittedData[],
										  double *amplitude, double *center,
										  double *stdDev, double *residue);
AnalysisLibErrType CVIFUNC LogFit        (const double x[], double y[], const double weight[],
										  ssize_t n, double logBase, int fitMethod,
										  double tolerance, double fittedData[],
										  double *amplitude, double *scale, double *residue);
AnalysisLibErrType CVIFUNC PowerFit      (const double x[], double y[], const double weight[],
										  ssize_t n, int fitMethod, double tolerance,
										  double fittedData[], double *amplitude,
										  double *power, double *residue);

AnalysisLibErrType CVIFUNC LinearFitEx   (const double x[], double y[], const double weight[],
										  ssize_t n, int fitMethod, double tolerance,
										  double fittedData[], double *slope,
										  double *intercept, double *residue);
AnalysisLibErrType CVIFUNC ExpFitEx      (const double x[], double y[], const double weight[],
										  ssize_t n, int fitMethod, double tolerance,
										  double fittedData[], double *amplitude,
										  double *damping, double *residue);
AnalysisLibErrType CVIFUNC PolyFitEx     (double x[], const double y[], ssize_t n, int order,
										  const int specOrder[], const double specCoef[], int nSpec,
                                          int algorithm,
										  double fittedData[], double coef[], double *mse);
AnalysisLibErrType CVIFUNC PolyFitWithWeight (double x[], const double y[],
					      const double weight[], ssize_t n,
					      int order, const int specOrder[],
					      const double specCoef[], int nSpec,
					      int algorithm,
					      double fittedData[], double coef[], double *mse);
AnalysisLibErrType CVIFUNC GoodnessOfFit (const double y[], const double fittedData[],
										  const double weight[], ssize_t n, ssize_t degOfFree,
										  double *SSE, double *rSquare, double *RMSE);
AnalysisLibErrType CVIFUNC CubicSplineFit(const double x[], double y[], const double weight[],
										  ssize_t n, double balance, const double smoothness[], double fittedData[]);

AnalysisLibErrType CVIFUNC LinearFitInterval(const double x[], const double y[],
											 const double weight[], ssize_t n,
											 int intervalType, double confidenceLevel,
											 double slope, double intercept,
											 double upperBound[], double lowerBound[],
											 double *deltaSlope, double *deltaIntcpt);
AnalysisLibErrType CVIFUNC ExpFitInterval   (const double x[], const double y[],
											 const double weight[], ssize_t n,
										     int intervalType, double confidenceLevel,
										     double amplitude, double damping,
											 double upperBound[], double lowerBound[],
											 double *deltaAmp, double *deltaDamp);
AnalysisLibErrType CVIFUNC GaussFitInterval (const double x[], const double y[],
											 const double weight[], ssize_t n,
											 int intervalType, double confidenceLevel,
											 double amplitude, double center,
											 double stdDev, double upperBound[],
											 double lowerBound[], double *deltaAmp,
											 double *deltaCnt, double *deltaDev);
AnalysisLibErrType CVIFUNC LogFitInterval   (const double x[], const double y[],
											 const double weight[], ssize_t n,
							  				 double logBase, int intervalType,
							  				 double confidenceLevel, double amplitude,
							  				 double scale, double upperBound[],
											 double lowerBound[], double *deltaAmp,
											 double *deltaScl);
AnalysisLibErrType CVIFUNC PowerFitInterval (const double x[], const double y[],
											 const double weight[], ssize_t n,
											 int intervalType, double confidenceLevel,
											 double amplitude, double power,
											 double upperBound[], double lowerBound[],
											 double *deltaAmp, double *deltaPow);

AnalysisLibErrType CVIFUNC RemoveOutlierByIndex (double x[], double y[],
												 double weight[], ssize_t *n,
												 const ssize_t index[], ssize_t nIndex);
AnalysisLibErrType CVIFUNC RemoveOutlierByRange (double x[], double y[],
												 double weight[], ssize_t *n,
												 int rangeType, const double range[],
												 int nRange, ssize_t outlierIndex[],
												 ssize_t *nOutlier);

/*- Old-style Curve Fitting Function (replaced by GenLSFit) -*/

AnalysisLibErrType CVIFUNC GenLSFitCoef (const void *H, ssize_t row, ssize_t col,
                                         const double y[], double coef[],
                                         int algorithm);


/*- Interpolation ------------------------------------------------------------*/

AnalysisLibErrType CVIFUNC PolyInterp (const double x[], const double y[],
                                       ssize_t n, double x_val, double *Interp_Val,
                                       double *Error);
AnalysisLibErrType CVIFUNC RatInterp  (const double x[], const double y[],
                                       ssize_t n, double x_val, double *Interp_Val,
                                       double *Error);
AnalysisLibErrType CVIFUNC SpInterp   (const double x[], const double y[],
                                       const double y2[], ssize_t n, double x_val,
                                       double *Interp_Val);
AnalysisLibErrType CVIFUNC Spline     (const double x[], const double y[],
                                       ssize_t n, double b1, double b2, double y2[]);


/*- Vector and Matrix Algebra ------------------------------------------------*/
/*- Real Matrices ----------------*/

AnalysisLibErrType CVIFUNC SpecialMatrix (int type, ssize_t m, const double x[], ssize_t nx,
										  const double y[], ssize_t ny, void *z);
AnalysisLibErrType CVIFUNC DotProduct    (const double x[], const double y[],
                                          ssize_t n, double *dotProd);
AnalysisLibErrType CVIFUNC Transpose     (const void *x, ssize_t n, ssize_t m, void *y);
AnalysisLibErrType CVIFUNC Determinant   (const void *x, ssize_t n, double *determinant);
AnalysisLibErrType CVIFUNC GenDeterminant(void *a, ssize_t n, int type, double *determinant);
AnalysisLibErrType CVIFUNC Trace         (const void *x, ssize_t n, double *trace);
AnalysisLibErrType CVIFUNC InvMatrix     (const void *x, ssize_t n, void *y);
AnalysisLibErrType CVIFUNC GenInvMatrix  (void *a, ssize_t n, int type, void *mi);
AnalysisLibErrType CVIFUNC LinEqs        (const void *A, const double y[],
                                          ssize_t n, double x[]);
AnalysisLibErrType CVIFUNC GenLinEqs     (void *a, ssize_t n, ssize_t m, const double y[], int type,
									      double x[]);
AnalysisLibErrType CVIFUNC MatrixMul     (const void *x, const void *y, ssize_t n,
                                          ssize_t k, ssize_t m, void *z);
AnalysisLibErrType CVIFUNC MatrixVectorMul (const void *A, const double x[],
                                            ssize_t n, ssize_t m, double y[]);
AnalysisLibErrType CVIFUNC OuterProduct  (const double vectorX[], ssize_t nx,
                                 		  const double vectorY[], ssize_t ny,
                                 		  void *z);
AnalysisLibErrType CVIFUNC MatrixRank    (void *a, ssize_t n, ssize_t m, double tolerance, ssize_t *rank);
AnalysisLibErrType CVIFUNC MatrixNorm    (void *a, ssize_t n, ssize_t m, int normType, double *norm);
AnalysisLibErrType CVIFUNC ConditionNumber     (void *a, ssize_t n, ssize_t m, int normType,
												double *conditionNumber);
AnalysisLibErrType CVIFUNC SymEigenValueVector (void *a, ssize_t n, int outputChoice,
												double eigenValues[], void *eigenVectors);
AnalysisLibErrType CVIFUNC GenEigenValueVector (void *a, ssize_t n, int outputChoice,
										        NIComplexNumber eigenValues[], void *eigenVectors);

AnalysisLibErrType CVIFUNC SVDS 		 (void *a, ssize_t n, ssize_t m, double s[]);
AnalysisLibErrType CVIFUNC SVD           (void *a, ssize_t n, ssize_t m, void *u, double s[], void *v);
AnalysisLibErrType CVIFUNC QR 			 (void *a, ssize_t n, ssize_t m, int algorithm,
										  void *q, void *r);
AnalysisLibErrType CVIFUNC Cholesky      (void *a, ssize_t n, void *r);
AnalysisLibErrType CVIFUNC PseudoInverse (void *a, ssize_t n, ssize_t m, double tolerance,
										  void *mpi);
AnalysisLibErrType CVIFUNC CheckPosDef   (void *a, ssize_t n, int *positiveDefinite);
AnalysisLibErrType CVIFUNC LU            (void *a, ssize_t n, ssize_t p[], int *sign);
AnalysisLibErrType CVIFUNC ForwSub       (const void *a, const double y[],
                                          ssize_t n, double x[], const ssize_t p[]);
AnalysisLibErrType CVIFUNC BackSub       (const void *a, const double y[],
                                          ssize_t n, double x[]);

AnalysisLibErrType CVIFUNC SVDEx         (const void *a, ssize_t n, ssize_t m,
										  int sizeOption, void *u, double s[], void *v);
AnalysisLibErrType CVIFUNC QREx          (void *a, ssize_t n, ssize_t m, int pvt,
										  int sizeOption, void *pvtInfo, void *q, void *r);
AnalysisLibErrType CVIFUNC MatrixBalance (void *a, ssize_t n, int method, ssize_t *ilo,
										  ssize_t *ihi, double scale[], void *b);
AnalysisLibErrType CVIFUNC SolveEqs            (const void *a, ssize_t n, ssize_t m,
										        int typeA, void *y, ssize_t ny, void *x);
AnalysisLibErrType CVIFUNC EigenVBack          (void *inV, ssize_t n, int side, int method,
										        ssize_t ilo, ssize_t ihi, const double scale[],
										        void *outV);
AnalysisLibErrType CVIFUNC GenEigenAB          (const void *a, const void *b, ssize_t n,
										        NIComplexNumber eigenvalues[], void *leftVec,
										        void *rightVec);
AnalysisLibErrType CVIFUNC Hess                (void *a, ssize_t n, void *h, void *q);
AnalysisLibErrType CVIFUNC QZ                  (const void *a, const void *b, ssize_t n, int order,
										        void *aa, void *bb, void *q, void *z,
										        NIComplexNumber alpha[], double beta[],
										        void *leftVec, void *rightVec);
AnalysisLibErrType CVIFUNC Schur               (const void *a, ssize_t n, int order,
										        void *q, void *s, NIComplexNumber eigenvalues[]);
AnalysisLibErrType CVIFUNC KroneckerProd (const void *x, const void *y,
					  ssize_t n, ssize_t m, ssize_t k, ssize_t l,
					  void *KronProd);
AnalysisLibErrType CVIFUNC UnitVector(double x[], ssize_t n, int normType,
	  								  double userdefinednorm, double *norm);
AnalysisLibErrType CVIFUNC AutoCorrMtrx(double x[], ssize_t nx, ssize_t order,
										int method, void *autocorrmtrx);


/*- Complex Matrices ---------------------*/

AnalysisLibErrType CVIFUNC CxSpecialMatrix     (int matrixType, ssize_t m, const NIComplexNumber x[], ssize_t nx,
												const NIComplexNumber y[], ssize_t ny, void *z);
AnalysisLibErrType CVIFUNC CxDotProduct        (const NIComplexNumber x[], const NIComplexNumber y[],
												ssize_t n, NIComplexNumber *dotProduct);
AnalysisLibErrType CVIFUNC CxTranspose         (void *x, ssize_t n, ssize_t m, void *y);
AnalysisLibErrType CVIFUNC CxDeterminant       (void *x, ssize_t n, int matrixType, NIComplexNumber *determinant);
AnalysisLibErrType CVIFUNC CxTrace 			   (void *x, ssize_t n, int reserved, NIComplexNumber *trace);
AnalysisLibErrType CVIFUNC CxGenInvMatrix      (void *a, ssize_t n, int matrixType, void *mi);
AnalysisLibErrType CVIFUNC CxGenLinEqs         (void *a, ssize_t n, ssize_t m, const NIComplexNumber y[], int matrixType,
												NIComplexNumber x[]);
AnalysisLibErrType CVIFUNC CxMatrixMul         (const void *x, const void *y, ssize_t n,
												ssize_t k, ssize_t m, void *z);
AnalysisLibErrType CVIFUNC CxMatrixVectorMul (const void *A, const NIComplexNumber x[],
                                              ssize_t n, ssize_t m, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxOuterProduct      (const NIComplexNumber vectorX[], ssize_t nx,
												const NIComplexNumber vectorY[], ssize_t ny,
												void *outerProduct);
AnalysisLibErrType CVIFUNC CxMatrixRank        (void *a, ssize_t n, ssize_t m, double tolerance, ssize_t *rank);
AnalysisLibErrType CVIFUNC CxMatrixNorm 	   (void *a, ssize_t n, ssize_t m, int normType, double *norm);
AnalysisLibErrType CVIFUNC CxConditionNumber   (void *a, ssize_t n, ssize_t m, int normType,
												double *conditionNumber);
AnalysisLibErrType CVIFUNC CxEigenValueVector  (void *a, ssize_t n, int matrixType, int outputChoice,
                                                NIComplexNumber eigenValues[], void *eigenVectors);
AnalysisLibErrType CVIFUNC CxSVDS 			   (void *a, ssize_t n, ssize_t m, NIComplexNumber s[]);
AnalysisLibErrType CVIFUNC CxSVD               (void *a, ssize_t n, ssize_t m, void *u, NIComplexNumber s[], void *v);
AnalysisLibErrType CVIFUNC CxQR 			   (void *a, ssize_t n, ssize_t m, void *q, void *r);
AnalysisLibErrType CVIFUNC CxCholesky          (void *a, ssize_t n, void *r);
AnalysisLibErrType CVIFUNC CxPseudoInverse	   (void *a, ssize_t n, ssize_t m, double tolerance,
												void *mpi);
AnalysisLibErrType CVIFUNC CxCheckPosDef 	   (void *a, ssize_t n, int *positiveDefinite);
AnalysisLibErrType CVIFUNC CxLU 			   (void *a, ssize_t n, ssize_t p[], int *sign);

AnalysisLibErrType CVIFUNC CxSVDEx             (const void *a, ssize_t n, ssize_t m,
												int sizeOption, void *u, double s[], void *v);
AnalysisLibErrType CVIFUNC CxQREx              (void *a, ssize_t n, ssize_t m,
												int pvt, int sizeOption, void *pvtInfo,
												void *q, void *r);
AnalysisLibErrType CVIFUNC CxMatrixBalance     (void *a, ssize_t n, int method, ssize_t *ilo,
												ssize_t *ihi, double scale[], void *b);
AnalysisLibErrType CVIFUNC CxSolveEqs          (const void *a, ssize_t n, ssize_t m,
												int matrixType, void *y, ssize_t ny, void *x);
AnalysisLibErrType CVIFUNC CxEigenVBack        (void *inV, ssize_t n, int side, int method,
												ssize_t ilo, ssize_t ihi, const double scale[],
												void *outV);
AnalysisLibErrType CVIFUNC CxGenEigenAB        (const void *a, const void *b, ssize_t n,
												NIComplexNumber eigenvalues[], void *leftVec,
												void *rightVec);
AnalysisLibErrType CVIFUNC CxHess              (void *a, ssize_t n, void *h, void *q);
AnalysisLibErrType CVIFUNC CxQZ                (const void *a, const void *b, ssize_t n, int order,
												void *aa, void *bb, void *q, void *z,
												NIComplexNumber alpha[], NIComplexNumber beta[],
												void *leftVec, void *rightVec);
AnalysisLibErrType CVIFUNC CxSchur             (const void *a, ssize_t n, int order,
												void *q, void *s, NIComplexNumber eigenvalues[]);
AnalysisLibErrType CVIFUNC CxKroneckerProd (const void *x, const void *y,
				  ssize_t n, ssize_t m, ssize_t k, ssize_t l, void *KronProd);
AnalysisLibErrType CVIFUNC CxUnitVector(NIComplexNumber x[], ssize_t n, int normType,
										double userdefinednorm, double *norm);
AnalysisLibErrType CVIFUNC CxAutoCorrMtrx(NIComplexNumber x[], ssize_t nx, ssize_t order,
										  int method, void *autocorrmtrx);


/*- Additional Numeric Functions ---------------------------------------------*/

AnalysisLibErrType CVIFUNC PolyRootsEx  (const double     x[], int n, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxPolyRoots  (const double     x[], int n, NIComplexNumber y[]);
AnalysisLibErrType CVIFUNC CxPolyRootsEx(const NIComplexNumber x[], int n, NIComplexNumber y[]);

AnalysisLibErrType CVIFUNC NumericIntegration (const double x[], ssize_t n, double dt,
									           int integrationMethod, double *ir);
AnalysisLibErrType CVIFUNC PeakDetector (const double x[], ssize_t n, double threshold, ssize_t width,
								         int polarity, int initialize, int endOfData, ssize_t *count,
                                         double **pL, double **pA, double **pSD);
AnalysisLibErrType CVIFUNC ThresholdPeakDetector (const double x[], ssize_t n, double threshold,
												  ssize_t width, ssize_t peakIndices[], ssize_t *count);

/* Special Functions */

void   CVIFUNC Airy             (double x, double *ai, double *bi);
double CVIFUNC Bessel1st        (double r, double x);
double CVIFUNC Bessel2nd        (double r, double x);
double CVIFUNC SphBessel1st     (int n, double x);
double CVIFUNC SphBessel2nd     (int n, double x);
double CVIFUNC ModBessel1st     (double r, double x);
double CVIFUNC ModBessel2nd     (int n, double x);
double CVIFUNC Beta             (double x, double y, double *a);
double CVIFUNC Fact             (int n);
double CVIFUNC Gamma            (double x, double *a);
double CVIFUNC GammaC           (double x, double a);
double CVIFUNC LnFact           (int n);
double CVIFUNC LnGamma          (double x);
double CVIFUNC Psi              (double x);
double CVIFUNC Stirling         (double x);
double CVIFUNC GaussHG          (double x, double a, double b, double c);
double CVIFUNC GaussHypergeometric(double x, double a, double b, double c);
double CVIFUNC Kummer           (double x, double a, double b);
double CVIFUNC Tricomi          (double x, double a, double b);
double CVIFUNC Elliptic1st      (double k, double *a);
double CVIFUNC Elliptic2nd      (double k, double *a);
double CVIFUNC Dawson           (double x);
void   CVIFUNC FresnelIntegrals (double x, double *fSinI, double *fCosI);
double CVIFUNC ExpIntegral      (double x, int n);
double CVIFUNC CosIntegral      (double x);
double CVIFUNC SinIntegral      (double x);
double CVIFUNC CoshIntegral     (double x);
double CVIFUNC SinhIntegral     (double x);
void   CVIFUNC JacobiEllipticI  (double u, double k, double *cn, double *sn,
								 double *dn, double *phi);
void   CVIFUNC Kelvin1st        (double x, int n, NIComplexNumber *be);
void   CVIFUNC Kelvin2nd        (double x, int n, NIComplexNumber *ke);
double CVIFUNC Dilogarithm      (double x);
double CVIFUNC ParabolicCylinder(double x, double v);
double CVIFUNC Struve           (double x, double v);
double CVIFUNC Zeta             (double x);


/*- Error String -------------------------------------------------------------*/

void CVIFUNC FreeAnalysisMem (void *pointer);

char * CVIFUNC GetAnalysisErrorString(int errorNum);

/*- The End ------------------------------------------------------------------*/

#ifdef __cplusplus
 }
#endif

#endif /* !defined(_ANALYSIS_H) */
