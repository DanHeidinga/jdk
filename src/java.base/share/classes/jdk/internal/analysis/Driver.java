package jdk.internal.analysis;

import java.lang.reflect.Method;
import java.util.Objects;
import java.util.Scanner;
import java.util.Set;
import java.util.HashSet;

public class Driver {
	
	private static native void registerNatives();
    static {
        registerNatives();
    }

	
	// TODO: use records?
	class MethodTask {
		final Class<?> holder;
		final long methodPtr;
		final int callingBytecode; // one of { invokevirtual, invokespecial, invokestatic, invokeinterface }

		public MethodTask(Class<?> holder, long method, int bytecode) {
			this.holder = holder;
			this.methodPtr = method;
			this.callingBytecode = bytecode;
		}

		public boolean equals(Object o) {
			if (o == this) { return true; }
			if (o instanceof MethodTask) {
				MethodTask m = (MethodTask) o;
				if (this.holder == m.holder && this.methodPtr == m.methodPtr && this.callingBytecode == m.callingBytecode) {
					return true;
				}
			}
			return false;
		}
		
		public int hashCode() {
			return (int)((((methodPtr >> 2 /* remove low 00 bits */) * 19 ) + callingBytecode) * 19 + holder.hashCode());
		}
	}

	Set<MethodTask> methodQueue = new HashSet<MethodTask>();
	Set<MethodTask> processedMethods = new HashSet<MethodTask>();
	/**
	 * Enqueue a method on the methodQueue.
	 * 
	 * @param holder the definiing class of the method
	 * @param method a pointer to a Method MetaData
	 * @param bytecode the bytecode used to call the method
	 */
	public synchronized void enqueueMethod(Class<?> holder, long method, int bytecode) {
		MethodTask mt = new MethodTask(holder, method, bytecode);
		if (!processedMethods.contains(mt)) {
			methodQueue.add(mt);
		}
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
