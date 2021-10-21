/*
 * Copyright (c) 1999, 2021, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "ci/bcReachabilityAnalyzer.hpp"
#include "classfile/symbolTable.hpp"
#include "compiler/compilerDirectives.hpp"
#include "memory/oopFactory.hpp"
#include "runtime/handles.inline.hpp"
#include "runtime/interfaceSupport.inline.hpp"
#include "runtime/javaCalls.hpp"
#include "staticanalyzer/saJavaClasses.hpp"
#include "staticanalyzer/staticAnalyzer.hpp"

void StaticAnalyzer::initialize() {
  JavaThread* THREAD = JavaThread::current();
  jdk_internal_staticanalysis_StaticAnalyzer::compute_offsets(THREAD); // should this be moved to saJavaClasses.cpp
}

void StaticAnalyzer::compile_method(ciEnv* env, ciMethod* target, int entry_bci, bool install_code, DirectiveSet* directive) {
  JavaThread* THREAD = JavaThread::current();
  printf("StaticAnalyzer::compile_method>>>\n");
  // do reachability analysis
  BCReachabilityAnalyzer *bcra = target->get_bcra();
printf("# bcra computed\n");
  // TODO: load classes discovered by bcra
  {
    ThreadInVMfromUnknown tiv;
    ResetNoHandleMark rnhm;
    Klass *accessing_klass = target->get_Method()->constants()->pool_holder(); /// TODO
    Handle class_loader = Handle(THREAD, accessing_klass->class_loader());
    Handle protection_domain  = Handle(THREAD, accessing_klass->protection_domain());
    
    GrowableArray<ciType*> classes = bcra->get_classes();
    for (GrowableArrayIterator<ciType*> it = classes.begin(); it != classes.end(); ++it) {
      ciType *c = *it;
      ciKlass *k = c->as_klass();
      Klass *klazz = nullptr;
      if (c->is_loaded() && false) {
        // get_Klass() is protected, can't use here.  Force the lookup
        //klazz = k->get_Klass();
      } else {
        Symbol *name_symbol = SymbolTable::new_symbol(k->name()->as_utf8(), k->name()->utf8_length());
        klazz = SystemDictionary::resolve_or_null(name_symbol, class_loader, protection_domain, THREAD);
      }
      if (klazz != nullptr) {
        JavaValue result(T_VOID);
        JavaCallArguments args;
        args.push_oop(Handle(THREAD, klazz->java_mirror()));
        JavaCalls::call_static(&result, jdk_internal_staticanalysis_StaticAnalyzer::klass(), vmSymbols::addDiscoveredClass_name(), vmSymbols::class_void_signature(), &args, THREAD);
      }
    }
  

    //make upcall to add callees in the set of methods to be analyzed
    GrowableArray<ciMethod*> callees = bcra->get_callees();
    int numCallees = callees.length();
    if (numCallees > 0) {
      //ResetNoHandleMark rnhm;
      typeArrayOop calleeHandles = oopFactory::new_longArray(numCallees, CHECK);
      int i = 0;
      for (GrowableArrayIterator<ciMethod*> it = callees.begin(); it != callees.end(); ++it) {
        ciMethod *method = *it;
        if (method->is_loaded()) {
          jlong methodId = (jlong)method->get_Method();
          calleeHandles->long_at_put(i, methodId);
          i += 1;
        } else {
          printf("# TODO deal with unloaded methods\n");
        }
      }
      JavaValue result(T_VOID);
      JavaCallArguments args;
      args.push_oop(Handle(THREAD, calleeHandles));
      printf("# calling into Java\n");
      JavaCalls::call_static(&result, jdk_internal_staticanalysis_StaticAnalyzer::klass(), vmSymbols::addToQueue_name(), vmSymbols::addToQueueA_signature(), &args, THREAD);
    }
  }
  printf("<<< StaticAnalyzer::compile_method\n");
}

