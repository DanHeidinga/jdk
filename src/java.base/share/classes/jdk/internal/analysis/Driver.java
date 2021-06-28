package jdk.internal.analysis;

import java.lang.reflect.Method;
import java.util.Objects;
import java.util.Scanner;

public class Driver {
	
	private static native void registerNatives();
    static {
        registerNatives();
    }

	/**
	 * Entrypoint for the ci-based analysis engine.
	 * 
	 * args[0] = Class name of the main class
	 * 
	 * @param args the args to the engine
	 */
	public static void main(String[] args) throws Throwable {
		System.out.println("Hello driver");
		if (args.length < 1) {
			System.out.println("Please specify the main class");
			System.exit(-1);
		}

		System.out.println("Loading: ("+args[0]+")");
		Class<?> mainClass = Class.forName(args[0], false, ClassLoader.getSystemClassLoader());
		System.out.println("Loaded = " + mainClass);

		Driver driver = new Driver(mainClass);

		Method mainMethod = driver.getStartingPoint();

		Thread thread = driver.get_compiler_thread();
		System.out.println("===================== A =============");

		driver.analyze(mainMethod, thread);
		System.out.println("===================== B =============");
		Thread.sleep(10);

		Scanner sc = new Scanner(System.in);
		sc.next();
	}

	Class<?> mainClass;

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

	native Thread get_compiler_thread();

	native void analyze(Method m, Thread thread) throws Throwable;
}
