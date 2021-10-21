package jdk.internal.staticanalysis;

import java.lang.reflect.Method;
import java.util.Collections;
import java.util.Arrays;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentHashMap.KeySetView;
import java.util.stream.Collectors;

/**
 * Class is static for easy up-calls from native.
 */
final class StaticAnalyzer {
    static final Set<Long> workQueue = ConcurrentHashMap.newKeySet();

    static final Set<Class<?>> discoveredClasses = ConcurrentHashMap.newKeySet();
    

    public static void addDiscoveredClass(Class<?> c) {
        synchronized(discoveredClasses) {
            discoveredClasses.add(c);
        }
    }

    public static void addDiscoveredClasses(Class<?> c[]) {
        synchronized(discoveredClasses) {
            discoveredClasses.addAll(Arrays.asList(c));
        }
    }

    public static void addToQueue(long[] methodHandles) {
        long[] handlesToAdd = Arrays.stream(methodHandles).filter(handle -> workQueue.add(handle)).toArray();
        StringBuilder sb = new StringBuilder();
        sb.append("addToQueue: [");
        Arrays.stream(handlesToAdd).mapToObj(Long::toHexString).forEach(x -> sb.append(x).append(" "));
        System.out.println(sb.toString() + "]");
        analyze(handlesToAdd);
    }

    public static void addToQueue(long methodHandle) {
        long[] temp = {methodHandle};
        addToQueue(temp);
    }

    public static native boolean analyze(long[] methodHandles);
    
    public static native boolean enqueueReflectMethod(Method m);


    
    public static native void registerNatives();

    static {
        registerNatives();
    }
}
