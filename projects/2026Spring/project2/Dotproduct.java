import java.util.Random;

public class Dotproduct {
    private static final int WARMUP_RUNS = 5;
    private static final int MEASURE_RUNS = 5;
    private static final int[] SIZES = {1000, 10000, 100000, 1000000, 5000000, 10000000};
    private static final Random RANDOM = new Random(42L);

    private static volatile long sinkLong;
    private static volatile double sinkDouble;

    private static int randomIntRange(int min, int max) {
        return RANDOM.nextInt(max - min + 1) + min;
    }

    private static double randomUnitValue() {
        return -1.0 + 2.0 * RANDOM.nextDouble();
    }

    private static int[] createIntArray(int n) {
        int[] arr = new int[n];
        for (int i = 0; i < n; i++) {
            arr[i] = randomIntRange(-100, 100);
        }
        return arr;
    }

    private static short[] createShortArray(int n) {
        short[] arr = new short[n];
        for (int i = 0; i < n; i++) {
            arr[i] = (short) randomIntRange(-100, 100);
        }
        return arr;
    }

    private static byte[] createByteArray(int n) {
        byte[] arr = new byte[n];
        for (int i = 0; i < n; i++) {
            arr[i] = (byte) randomIntRange(-100, 100);
        }
        return arr;
    }

    private static float[] createFloatArray(int n) {
        float[] arr = new float[n];
        for (int i = 0; i < n; i++) {
            arr[i] = (float) randomUnitValue();
        }
        return arr;
    }

    private static double[] createDoubleArray(int n) {
        double[] arr = new double[n];
        for (int i = 0; i < n; i++) {
            arr[i] = randomUnitValue();
        }
        return arr;
    }

    private static long dotInt(int[] a, int[] b) {
        long sum = 0L;
        for (int i = 0; i < a.length; i++) {
            sum += (long) a[i] * (long) b[i];
        }
        return sum;
    }

    private static long dotShort(short[] a, short[] b) {
        long sum = 0L;
        for (int i = 0; i < a.length; i++) {
            sum += (long) a[i] * (long) b[i];
        }
        return sum;
    }

    private static long dotByte(byte[] a, byte[] b) {
        long sum = 0L;
        for (int i = 0; i < a.length; i++) {
            sum += (long) a[i] * (long) b[i];
        }
        return sum;
    }

    private static double dotFloat(float[] a, float[] b) {
        double sum = 0.0;
        for (int i = 0; i < a.length; i++) {
            sum += (double) a[i] * (double) b[i];
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

    private static void benchmarkInt(int n) {
        int[] a = createIntArray(n);
        int[] b = createIntArray(n);

        for (int i = 0; i < WARMUP_RUNS; i++) {
            sinkLong = dotInt(a, b);
        }

        long totalNs = 0L;
        long result = 0L;
        for (int i = 0; i < MEASURE_RUNS; i++) {
            long start = System.nanoTime();
            result = dotInt(a, b);
            long end = System.nanoTime();
            sinkLong = result;
            totalNs += (end - start);
        }

        System.out.printf("Java,int,%d,%d,%d%n", n, totalNs / MEASURE_RUNS, result);
    }

    private static void benchmarkShort(int n) {
        short[] a = createShortArray(n);
        short[] b = createShortArray(n);

        for (int i = 0; i < WARMUP_RUNS; i++) {
            sinkLong = dotShort(a, b);
        }

        long totalNs = 0L;
        long result = 0L;
        for (int i = 0; i < MEASURE_RUNS; i++) {
            long start = System.nanoTime();
            result = dotShort(a, b);
            long end = System.nanoTime();
            sinkLong = result;
            totalNs += (end - start);
        }

        System.out.printf("Java,short,%d,%d,%d%n", n, totalNs / MEASURE_RUNS, result);
    }

    private static void benchmarkByte(int n) {
        byte[] a = createByteArray(n);
        byte[] b = createByteArray(n);

        for (int i = 0; i < WARMUP_RUNS; i++) {
            sinkLong = dotByte(a, b);
        }

        long totalNs = 0L;
        long result = 0L;
        for (int i = 0; i < MEASURE_RUNS; i++) {
            long start = System.nanoTime();
            result = dotByte(a, b);
            long end = System.nanoTime();
            sinkLong = result;
            totalNs += (end - start);
        }

        System.out.printf("Java,byte,%d,%d,%d%n", n, totalNs / MEASURE_RUNS, result);
    }

    private static void benchmarkFloat(int n) {
        float[] a = createFloatArray(n);
        float[] b = createFloatArray(n);

        for (int i = 0; i < WARMUP_RUNS; i++) {
            sinkDouble = dotFloat(a, b);
        }

        long totalNs = 0L;
        double result = 0.0;
        for (int i = 0; i < MEASURE_RUNS; i++) {
            long start = System.nanoTime();
            result = dotFloat(a, b);
            long end = System.nanoTime();
            sinkDouble = result;
            totalNs += (end - start);
        }

        System.out.printf("Java,float,%d,%d,%.10f%n", n, totalNs / MEASURE_RUNS, result);
    }

    private static void benchmarkDouble(int n) {
        double[] a = createDoubleArray(n);
        double[] b = createDoubleArray(n);

        for (int i = 0; i < WARMUP_RUNS; i++) {
            sinkDouble = dotDouble(a, b);
        }

        long totalNs = 0L;
        double result = 0.0;
        for (int i = 0; i < MEASURE_RUNS; i++) {
            long start = System.nanoTime();
            result = dotDouble(a, b);
            long end = System.nanoTime();
            sinkDouble = result;
            totalNs += (end - start);
        }

        System.out.printf("Java,double,%d,%d,%.10f%n", n, totalNs / MEASURE_RUNS, result);
    }

    public static void main(String[] args) {
        System.out.println("language,type,n,avg_ns,result");
        for (int n : SIZES) {
            benchmarkInt(n);
            benchmarkShort(n);
            benchmarkByte(n);
            benchmarkFloat(n);
            benchmarkDouble(n);
        }
    }
}
