package jdk.internal.staticanalysis;

import java.lang.reflect.Method;
import java.util.Objects;
import java.util.Scanner;
import java.util.Set;
import java.util.HashSet;
import jdk.internal.staticanalysis.StaticAnalyzer;

public class Driver {
    
    /**
     * Entrypoint for the ci-based analysis engine.
     * 
     * args[0] = Class name of the main class
     * 
     * @param args the args to the engine
     */
    public static void main(String[] args) throws Throwable {
        System.out.println("=== Analysis Driver ===");
        if (args.length < 1) {
            System.out.println("Please specify the main class");
            System.exit(-1);
        }

        System.out.println("Loading: ("+args[0]+")");
        Class<?> mainClass = Class.forName(args[0], false, ClassLoader.getSystemClassLoader());
        System.out.println("Loaded: " + mainClass);

        Driver driver = new Driver(mainClass);

        Method mainMethod = driver.getStartingPoint();

        System.out.println("===================== A =============");

		StaticAnalyzer.enqueueReflectMethod(mainMethod);

        System.out.println("===================== B =============");
        Thread.sleep(10);

        Scanner sc = new Scanner(System.in);
        sc.next();
    }

    private Class<?> mainClass;

	

    Driver(Class<?> aClass) {
        mainClass = Objects.requireNonNull(aClass);
    }

    /**
     * Get the "main" method to start the analysis from
     * 
     * @return a j.l.r.Method for mainClass.main(String[])
     */
    Method getStartingPoint() throws Throwable {
        return mainClass.getDeclaredMethod("main", String[].class);
    }
}