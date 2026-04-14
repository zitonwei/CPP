import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Random;

public class Dotproduct {
    private static final int WARMUP_RUNS = 5;
    private static final int MEASURE_RUNS = 10;
    private static final int[] SIZES = {1000, 10000, 100000, 1000000, 5000000, 10000000};
    private static final Random RANDOM = new Random(42L);

    private enum FillMode {
        SAFE("safe"),
        OVERFLOW("overflow");

        final String label;

        FillMode(String label) {
            this.label = label;
        }
    }

    private static volatile int sinkInt;
    private static volatile short sinkShort;
    private static volatile byte sinkByte;
    private static volatile float sinkFloat;
    private static volatile double sinkDouble;

    private static PrintWriter csvWriter;

    private static double safeBoundFromMax(double maxValue, int n) {
        return (n <= 0 || maxValue <= 0.0) ? 1.0 : Math.sqrt(maxValue / n);
    }

    private static int clampIntBound(double bound, int maxValue) {
        if (bound < 1.0) {
            return 1;
        }
        if (bound > maxValue) {
            return maxValue;
        }
        return (int) bound;
    }

    private static int randomIntRange(int min, int max) {
        return RANDOM.nextInt(max - min + 1) + min;
    }

    private static int randomSign() {
        return RANDOM.nextBoolean() ? 1 : -1;
    }

    private static int[] createIntArray(int n, FillMode mode) {
        int minValue = clampIntBound(safeBoundFromMax(Integer.MAX_VALUE, n), Integer.MAX_VALUE);
        int[] arr = new int[n];
        for (int i = 0; i < n; i++) {
            if (mode == FillMode.SAFE) {
                arr[i] = randomIntRange(-minValue, minValue);
            } else {
                arr[i] = randomSign() * randomIntRange(minValue, Integer.MAX_VALUE);
            }
        }
        return arr;
    }

    private static short[] createShortArray(int n, FillMode mode) {
        int minValue = clampIntBound(safeBoundFromMax(Short.MAX_VALUE, n), Short.MAX_VALUE);
        short[] arr = new short[n];
        for (int i = 0; i < n; i++) {
            if (mode == FillMode.SAFE) {
                arr[i] = (short) randomIntRange(-minValue, minValue);
            } else {
                arr[i] = (short) (randomSign() * randomIntRange(minValue, Short.MAX_VALUE));
            }
        }
        return arr;
    }

    private static byte[] createByteArray(int n, FillMode mode) {
        int minValue = clampIntBound(safeBoundFromMax(Byte.MAX_VALUE, n), Byte.MAX_VALUE);
        byte[] arr = new byte[n];
        for (int i = 0; i < n; i++) {
            if (mode == FillMode.SAFE) {
                arr[i] = (byte) randomIntRange(-minValue, minValue);
            } else {
                arr[i] = (byte) (randomSign() * randomIntRange(minValue, Byte.MAX_VALUE));
            }
        }
        return arr;
    }

    private static float[] createFloatArray(int n) {
        float[] arr = new float[n];
        for (int i = 0; i < n; i++) {
            arr[i] = -1.0f + 2.0f * RANDOM.nextFloat();
        }
        return arr;
    }

    private static double[] createDoubleArray(int n) {
        double[] arr = new double[n];
        for (int i = 0; i < n; i++) {
            arr[i] = -1.0 + 2.0 * RANDOM.nextDouble();
        }
        return arr;
    }

    private static int dotInt(int[] a, int[] b) {
        int sum = 0;
        for (int i = 0; i < a.length; i++) {
            sum += a[i] * b[i];
        }
        return sum;
    }

    private static short dotShort(short[] a, short[] b) {
        short sum = 0;
        for (int i = 0; i < a.length; i++) {
            sum += (short) (a[i] * b[i]);
        }
        return sum;
    }

    private static byte dotByte(byte[] a, byte[] b) {
        byte sum = 0;
        for (int i = 0; i < a.length; i++) {
            sum += (byte) (a[i] * b[i]);
        }
        return sum;
    }

    private static float dotFloat(float[] a, float[] b) {
        float sum = 0.0f;
        for (int i = 0; i < a.length; i++) {
            sum += a[i] * b[i];
        }
        return sum;
    }

    private static double dotDouble(double[] a, double[] b) {
        double sum = 0.0;
        for (int i = 0; i < a.length; i++) {
            sum += a[i] * b[i];
        }
        return sum;
    }

    private static void printHeader() {
        System.out.printf("%-8s %-10s %-12s %10s %16s %16s %20s%n",
            "language", "mode", "type", "n", "warmup_avg_ns", "measure_avg_ns", "result");
        csvWriter.println("language,mode,type,n,warmup_avg_ns,measure_avg_ns,result");
    }

    private static void printIntegerRow(String type, FillMode mode, int n,
            long warmupAvgNs, long measureAvgNs, int result) {
        System.out.printf("%-8s %-10s %-12s %10d %16d %16d %20d%n",
            "Java", mode.label, type, n, warmupAvgNs, measureAvgNs, result);
        csvWriter.printf("Java,%s,%s,%d,%d,%d,%d%n",
            mode.label, type, n, warmupAvgNs, measureAvgNs, result);
    }

    private static void printFpRow(String type, int n,
            long warmupAvgNs, long measureAvgNs, double result) {
        System.out.printf("%-8s %-10s %-12s %10d %16d %16d %20.10e%n",
            "Java", "safe", type, n, warmupAvgNs, measureAvgNs, result);
        csvWriter.printf("Java,safe,%s,%d,%d,%d,%.10e%n",
            type, n, warmupAvgNs, measureAvgNs, result);
    }

    private static void benchmarkInt(int n) {
        int[] aSafe = createIntArray(n, FillMode.SAFE);
        int[] bSafe = createIntArray(n, FillMode.SAFE);
        int[] aOverflow = createIntArray(n, FillMode.OVERFLOW);
        int[] bOverflow = createIntArray(n, FillMode.OVERFLOW);
        long[] warmupTotal = {0L, 0L};
        long[] measureTotal = {0L, 0L};
        int[] warmupCount = {0, 0};
        int[] measureCount = {0, 0};
        int[] result = {0, 0};

        while (warmupCount[FillMode.SAFE.ordinal()] < WARMUP_RUNS
                || warmupCount[FillMode.OVERFLOW.ordinal()] < WARMUP_RUNS) {
            FillMode mode = RANDOM.nextBoolean() ? FillMode.SAFE : FillMode.OVERFLOW;
            if (warmupCount[mode.ordinal()] >= WARMUP_RUNS) {
                mode = mode == FillMode.SAFE ? FillMode.OVERFLOW : FillMode.SAFE;
            }
            long start = System.nanoTime();
            result[mode.ordinal()] = mode == FillMode.SAFE ? dotInt(aSafe, bSafe) : dotInt(aOverflow, bOverflow);
            long end = System.nanoTime();
            sinkInt = result[mode.ordinal()];
            warmupTotal[mode.ordinal()] += (end - start);
            warmupCount[mode.ordinal()]++;
        }

        while (measureCount[FillMode.SAFE.ordinal()] < MEASURE_RUNS
                || measureCount[FillMode.OVERFLOW.ordinal()] < MEASURE_RUNS) {
            FillMode mode = RANDOM.nextBoolean() ? FillMode.SAFE : FillMode.OVERFLOW;
            if (measureCount[mode.ordinal()] >= MEASURE_RUNS) {
                mode = mode == FillMode.SAFE ? FillMode.OVERFLOW : FillMode.SAFE;
            }
            long start = System.nanoTime();
            result[mode.ordinal()] = mode == FillMode.SAFE ? dotInt(aSafe, bSafe) : dotInt(aOverflow, bOverflow);
            long end = System.nanoTime();
            sinkInt = result[mode.ordinal()];
            measureTotal[mode.ordinal()] += (end - start);
            measureCount[mode.ordinal()]++;
        }

        printIntegerRow("int", FillMode.SAFE, n,
            warmupTotal[FillMode.SAFE.ordinal()] / WARMUP_RUNS,
            measureTotal[FillMode.SAFE.ordinal()] / MEASURE_RUNS,
            result[FillMode.SAFE.ordinal()]);
        printIntegerRow("int", FillMode.OVERFLOW, n,
            warmupTotal[FillMode.OVERFLOW.ordinal()] / WARMUP_RUNS,
            measureTotal[FillMode.OVERFLOW.ordinal()] / MEASURE_RUNS,
            result[FillMode.OVERFLOW.ordinal()]);
    }

    private static void benchmarkShort(int n) {
        short[] aSafe = createShortArray(n, FillMode.SAFE);
        short[] bSafe = createShortArray(n, FillMode.SAFE);
        short[] aOverflow = createShortArray(n, FillMode.OVERFLOW);
        short[] bOverflow = createShortArray(n, FillMode.OVERFLOW);
        long[] warmupTotal = {0L, 0L};
        long[] measureTotal = {0L, 0L};
        int[] warmupCount = {0, 0};
        int[] measureCount = {0, 0};
        short[] result = {0, 0};

        while (warmupCount[FillMode.SAFE.ordinal()] < WARMUP_RUNS
                || warmupCount[FillMode.OVERFLOW.ordinal()] < WARMUP_RUNS) {
            FillMode mode = RANDOM.nextBoolean() ? FillMode.SAFE : FillMode.OVERFLOW;
            if (warmupCount[mode.ordinal()] >= WARMUP_RUNS) {
                mode = mode == FillMode.SAFE ? FillMode.OVERFLOW : FillMode.SAFE;
            }
            long start = System.nanoTime();
            result[mode.ordinal()] = mode == FillMode.SAFE ? dotShort(aSafe, bSafe) : dotShort(aOverflow, bOverflow);
            long end = System.nanoTime();
            sinkShort = result[mode.ordinal()];
            warmupTotal[mode.ordinal()] += (end - start);
            warmupCount[mode.ordinal()]++;
        }

        while (measureCount[FillMode.SAFE.ordinal()] < MEASURE_RUNS
                || measureCount[FillMode.OVERFLOW.ordinal()] < MEASURE_RUNS) {
            FillMode mode = RANDOM.nextBoolean() ? FillMode.SAFE : FillMode.OVERFLOW;
            if (measureCount[mode.ordinal()] >= MEASURE_RUNS) {
                mode = mode == FillMode.SAFE ? FillMode.OVERFLOW : FillMode.SAFE;
            }
            long start = System.nanoTime();
            result[mode.ordinal()] = mode == FillMode.SAFE ? dotShort(aSafe, bSafe) : dotShort(aOverflow, bOverflow);
            long end = System.nanoTime();
            sinkShort = result[mode.ordinal()];
            measureTotal[mode.ordinal()] += (end - start);
            measureCount[mode.ordinal()]++;
        }

        printIntegerRow("short", FillMode.SAFE, n,
            warmupTotal[FillMode.SAFE.ordinal()] / WARMUP_RUNS,
            measureTotal[FillMode.SAFE.ordinal()] / MEASURE_RUNS,
            result[FillMode.SAFE.ordinal()]);
        printIntegerRow("short", FillMode.OVERFLOW, n,
            warmupTotal[FillMode.OVERFLOW.ordinal()] / WARMUP_RUNS,
            measureTotal[FillMode.OVERFLOW.ordinal()] / MEASURE_RUNS,
            result[FillMode.OVERFLOW.ordinal()]);
    }

    private static void benchmarkByte(int n) {
        byte[] aSafe = createByteArray(n, FillMode.SAFE);
        byte[] bSafe = createByteArray(n, FillMode.SAFE);
        byte[] aOverflow = createByteArray(n, FillMode.OVERFLOW);
        byte[] bOverflow = createByteArray(n, FillMode.OVERFLOW);
        long[] warmupTotal = {0L, 0L};
        long[] measureTotal = {0L, 0L};
        int[] warmupCount = {0, 0};
        int[] measureCount = {0, 0};
        byte[] result = {0, 0};

        while (warmupCount[FillMode.SAFE.ordinal()] < WARMUP_RUNS
                || warmupCount[FillMode.OVERFLOW.ordinal()] < WARMUP_RUNS) {
            FillMode mode = RANDOM.nextBoolean() ? FillMode.SAFE : FillMode.OVERFLOW;
            if (warmupCount[mode.ordinal()] >= WARMUP_RUNS) {
                mode = mode == FillMode.SAFE ? FillMode.OVERFLOW : FillMode.SAFE;
            }
            long start = System.nanoTime();
            result[mode.ordinal()] = mode == FillMode.SAFE ? dotByte(aSafe, bSafe) : dotByte(aOverflow, bOverflow);
            long end = System.nanoTime();
            sinkByte = result[mode.ordinal()];
            warmupTotal[mode.ordinal()] += (end - start);
            warmupCount[mode.ordinal()]++;
        }

        while (measureCount[FillMode.SAFE.ordinal()] < MEASURE_RUNS
                || measureCount[FillMode.OVERFLOW.ordinal()] < MEASURE_RUNS) {
            FillMode mode = RANDOM.nextBoolean() ? FillMode.SAFE : FillMode.OVERFLOW;
            if (measureCount[mode.ordinal()] >= MEASURE_RUNS) {
                mode = mode == FillMode.SAFE ? FillMode.OVERFLOW : FillMode.SAFE;
            }
            long start = System.nanoTime();
            result[mode.ordinal()] = mode == FillMode.SAFE ? dotByte(aSafe, bSafe) : dotByte(aOverflow, bOverflow);
            long end = System.nanoTime();
            sinkByte = result[mode.ordinal()];
            measureTotal[mode.ordinal()] += (end - start);
            measureCount[mode.ordinal()]++;
        }

        printIntegerRow("signed char", FillMode.SAFE, n,
            warmupTotal[FillMode.SAFE.ordinal()] / WARMUP_RUNS,
            measureTotal[FillMode.SAFE.ordinal()] / MEASURE_RUNS,
            result[FillMode.SAFE.ordinal()]);
        printIntegerRow("signed char", FillMode.OVERFLOW, n,
            warmupTotal[FillMode.OVERFLOW.ordinal()] / WARMUP_RUNS,
            measureTotal[FillMode.OVERFLOW.ordinal()] / MEASURE_RUNS,
            result[FillMode.OVERFLOW.ordinal()]);
    }

    private static void benchmarkFloat(int n) {
        float[] a = createFloatArray(n);
        float[] b = createFloatArray(n);
        long warmupTotal = 0L;
        long measureTotal = 0L;
        float result = 0.0f;

        for (int i = 0; i < WARMUP_RUNS; i++) {
            long start = System.nanoTime();
            result = dotFloat(a, b);
            long end = System.nanoTime();
            sinkFloat = result;
            warmupTotal += (end - start);
        }

        for (int i = 0; i < MEASURE_RUNS; i++) {
            long start = System.nanoTime();
            result = dotFloat(a, b);
            long end = System.nanoTime();
            sinkFloat = result;
            measureTotal += (end - start);
        }

        printFpRow("float", n, warmupTotal / WARMUP_RUNS, measureTotal / MEASURE_RUNS, result);
    }

    private static void benchmarkDouble(int n) {
        double[] a = createDoubleArray(n);
        double[] b = createDoubleArray(n);
        long warmupTotal = 0L;
        long measureTotal = 0L;
        double result = 0.0;

        for (int i = 0; i < WARMUP_RUNS; i++) {
            long start = System.nanoTime();
            result = dotDouble(a, b);
            long end = System.nanoTime();
            sinkDouble = result;
            warmupTotal += (end - start);
        }

        for (int i = 0; i < MEASURE_RUNS; i++) {
            long start = System.nanoTime();
            result = dotDouble(a, b);
            long end = System.nanoTime();
            sinkDouble = result;
            measureTotal += (end - start);
        }

        printFpRow("double", n, warmupTotal / WARMUP_RUNS, measureTotal / MEASURE_RUNS, result);
    }

    public static void main(String[] args) throws IOException {
        csvWriter = new PrintWriter(new FileWriter("java_results.csv"));
        printHeader();

        for (int n : SIZES) {
            benchmarkInt(n);
            benchmarkShort(n);
            benchmarkByte(n);
            benchmarkFloat(n);
            benchmarkDouble(n);
        }

        csvWriter.close();
    }
}
