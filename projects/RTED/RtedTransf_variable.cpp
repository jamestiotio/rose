/*
 * RtedTransf_variable.cpp
 *
 *  Created on: Jul 7, 2009
 *      Author: panas2
 */


#include <rose.h>
#include <string>
#include <boost/foreach.hpp>
#include "RtedSymbols.h"
#include "DataStructures.h"
#include "RtedTransformation.h"

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;


// ------------------------ VARIABLE SPECIFIC CODE --------------------------

bool RtedTransformation::isVarInCreatedVariables(SgInitializedName* n) {
  bool ret=false;
  ROSE_ASSERT(n);
  Rose_STL_Container<SgInitializedName*>::const_iterator it=variable_declarations.begin();
  for (;it!=variable_declarations.end();++it) {
    SgInitializedName* initName = *it;
    if (initName==n) {
      ret=true;
      break;
    }
  }
  return ret;
}

void RtedTransformation::visit_isSgVariableDeclaration(SgNode* n) {
  SgVariableDeclaration* varDecl = isSgVariableDeclaration(n);

  // FIXME 2 djh: perhaps n->get_parent should be n->get_scope ?
  // don't track members of user types (structs, classes)
  if(   isSgClassDefinition( varDecl -> get_parent() ))
      return;

  Rose_STL_Container<SgInitializedName*> vars = varDecl->get_variables();
  Rose_STL_Container<SgInitializedName*>::const_iterator it = vars.begin();
  cerr << " ...... CHECKING Variable Declaration " << endl;
  for (;it!=vars.end();++it) {
    SgInitializedName* initName = *it;
    ROSE_ASSERT(initName);

    SgType* var_type = initName -> get_type();
    // reference types don't create more memory as far as the RTS is
    // concerned (in that &foo == &bar for bar a ref. of foo)
    if( isSgReferenceType( var_type ))
        continue;

    // Consider, e.g.
    //  MyClass a;
    //  RuntimeSystem_createVariable( a )
    // Here we informed the RTS of memory allocation after the constructor was
    // already run, so we might easily run into false positives from the
    // instrumented constructors.
    if(     isSgClassType( var_type )
            && hasNonEmptyConstructor( isSgClassType( var_type )))
        continue;

    // need to get the type and the possible value that it is initialized with
    cerr << "      Detected initName : " << initName->unparseToString() ;
    cerr <<"  type : " << initName->get_type()->unparseToString() << endl;
    variable_declarations.push_back(initName);
  }
}


// FIXME 2 djh: This presently inserts erroneous calls to createvariable when
// multiple constructors are called for the same variable (e.g. superclass
// constructors).
void RtedTransformation::insertVariableCreateCall( RtedClassDefinition* rcdef ) {
    SgClassDefinition* cdef = rcdef -> classDef;

    SgDeclarationStatementPtrList constructors;
    appendConstructors( cdef, constructors );

    BOOST_FOREACH( SgDeclarationStatement* decl, constructors ) {
        SgMemberFunctionDeclaration* constructor 
            = isSgMemberFunctionDeclaration( decl );
        // validate the postcondition of appendConstructors
        ROSE_ASSERT( constructor );

        // FIXME 2: Probably the thing to do in this case is simply to bail and
        // trust that the constructor will get transformed when its definition
        // is processed
        SgFunctionDefinition* def = constructor -> get_definition();
        ROSE_ASSERT( def );

        SgBasicBlock* body = def -> get_body();

        SgSymbolTable* sym_tab 
            = cdef -> get_declaration() -> get_scope() -> get_symbol_table();

        SgClassSymbol* csym
            = isSgClassSymbol(
                    sym_tab -> find_class(
                        cdef -> get_declaration() -> get_name() ));
        ROSE_ASSERT( csym );

	SgExprStatement* expr = buildVariableCreateCallStmt( buildThisExp( csym ));
	if (expr)
        body -> prepend_statement(expr);
    }
}

void RtedTransformation::insertVariableCreateCall(SgInitializedName* initName
						  ) {
  SgStatement* stmt = getSurroundingStatement(initName);
  // make sure there is no extern in front of stmt
  bool externQual = isGlobalExternVariable(stmt);
  if (externQual) {
    cerr << "Skipping this insertVariableCreateCall because it probably occurs multiple times (with and without extern)." << endl;
    return;
  }


  if (isSgStatement(stmt)) {
    SgScopeStatement* scope = stmt->get_scope();
    string name = initName->get_mangled_name().str();

    ROSE_ASSERT(scope);
    // what if there is an array creation within a ClassDefinition
    if ( isSgClassDefinition(scope)) {
      // new stmt = the classdef scope
      SgClassDeclaration* decl = isSgClassDeclaration(scope->get_parent());
      ROSE_ASSERT(decl);
      stmt = isSgVariableDeclaration(decl->get_parent());
      if (!stmt) {
	cerr << " Error . stmt is unknown : " << decl->get_parent()->class_name() << endl;
	ROSE_ASSERT( false );
      }
      scope = scope->get_scope();
      // We want to insert the stmt before this classdefinition, if its still in a valid block
      cerr <<" ....... Found ClassDefinition Scope. New Scope is : " << scope->class_name() << "  stmt:" << stmt->class_name() <<endl;
    }
    // what is there is an array creation in a global scope
    else if (isSgGlobal(scope)) {
      cerr <<"RuntimeInstrumentation :: WARNING - Scope not handled!!! : " << name << " : " << scope->class_name() << endl;
      // We need to add this new statement to the beginning of main
      // get the first statement in main as stmt
      stmt = mainFirst;
      scope = stmt->get_scope();

    }
    // for( int i =0;
    // ForStmt .. ForInitStmt .. <stmt>
    if( isSgForInitStatement( stmt -> get_parent() )) {
        // we have to handle for statements separately, because of parsing
        // issues introduced by variable declarations in the for loop's
        // init statement
      SgFunctionCallExp* buildVar = buildVariableCreateCallExpr( initName, stmt );
      if (buildVar==NULL) return;
        prependPseudoForInitializerExpression(
					      buildVar,
            isSgForStatement( stmt -> get_parent() -> get_parent() )
        );
    } else if ( isNormalScope( scope )) {
      // insert new stmt (exprStmt) after (old) stmt
      SgExprStatement* exprStmt = buildVariableCreateCallStmt( initName, stmt);
      if (exprStmt) {
      cerr << "++++++++++++ stmt :"<<stmt << " mainFirst:"<<mainFirst<<
      "   initName->get_scope():"<<initName->get_scope() <<
      "   mainFirst->get_scope():"<<mainFirst->get_scope()<<endl;
      // FIXME 2: stmt == mainFirst is probably wrong for cases where the
      // statment we want to instrument really is the first one in main (and not
      // merely one in the global scope)
      if( stmt == mainFirst && initName->get_scope()!=mainFirst->get_scope()) {
		  mainBody -> prepend_statement( exprStmt );
        cerr << "+++++++ insert Before... "<<endl;
      } else {
        // insert new stmt (exprStmt) after (old) stmt
        insertStatementAfter(isSgStatement(stmt), exprStmt);
        cerr << "+++++++ insert After... "<<endl;
      }
      }
    }
    else if (isSgNamespaceDefinitionStatement(scope)) {
      cerr <<"RuntimeInstrumentation :: WARNING - Scope not handled!!! : " << name << " : " << scope->class_name() << endl;
    } else {
      cerr
	<< "RuntimeInstrumentation :: Surrounding Block is not Block! : "
	<< name << " : " << scope->class_name() << endl;
      ROSE_ASSERT(false);
    }
  } else {
    cerr
      << "RuntimeInstrumentation :: Surrounding Statement could not be found! "
      << stmt->class_name() << endl;
    ROSE_ASSERT(false);
  }


}


// convenience function
SgFunctionCallExp*
RtedTransformation::buildVariableCreateCallExpr( SgThisExp* exp, bool forceinit) {

    string debug_name( "this" );
    return buildVariableCreateCallExpr(
            // we want &(*this), sizeof(*this)
            buildPointerDerefExp( exp ), debug_name, forceinit);
}

// convenience function
SgFunctionCallExp*
RtedTransformation::buildVariableCreateCallExpr(
        SgInitializedName* initName, SgStatement* stmt, bool forceinit) {

    SgInitializer* initializer = initName->get_initializer();
    bool initb = initializer || forceinit;

    string debug_name = initName -> get_name();

    return buildVariableCreateCallExpr(
                buildVarRef( initName ), debug_name, initb );
}

SgFunctionCallExp*
RtedTransformation::buildVariableCreateCallExpr(
        SgExpression* var_ref, string& debug_name, bool initb) {
  // tps: I am not sure yet if this is needed
#if 0
  // if the variable is called "this", then we want to take the
  // right hand side value
  if (debug_name=="this") {
    return NULL;
#if 0
    SgArrowExp* arrowOp = isSgArrowExp(var_ref->get_parent());
    ROSE_ASSERT(arrowOp);
    SgVarRefExp* newVarRef = isSgVarRefExp(arrowOp->get_rhs_operand());
    cerr << " ++++++ This FOUND! ++  Changing this : " << var_ref << "  to : " << newVarRef << endl;
    ROSE_ASSERT(newVarRef);
    var_ref=newVarRef;
#endif
  }
#endif    

    // build the function call : runtimeSystem-->createArray(params); ---------------------------
    SgExprListExp* arg_list = buildExprListExp();
    SgExpression* callName = buildString( debug_name );
    SgExpression* callNameExp = buildString( debug_name );

    SgExpression* initBool = buildIntVal(0);
    if (initb)
      initBool = buildIntVal(1);

    appendExpression(arg_list, callName);
    appendExpression(arg_list, callNameExp);
    appendTypeInformation( var_ref -> get_type(), arg_list );

    if( isSgVarRefExp( var_ref )) {
        appendAddressAndSize(
            isSgVarRefExp( var_ref ) -> get_symbol() -> get_declaration(),
            isSgVarRefExp(var_ref),
            arg_list,
            0 );
    } else {
        appendAddressAndSize( var_ref, var_ref -> get_type(), arg_list, 0 );
    }


    appendExpression(arg_list, initBool);
	appendClassName( arg_list, var_ref -> get_type() );

    SgExpression* filename = buildString(var_ref->get_file_info()->get_filename());
    int currentlinenr = var_ref->get_file_info()->get_line();
    SgExpression* linenr = buildString(RoseBin_support::ToString(currentlinenr));
    appendExpression(arg_list, linenr);
    appendExpression(arg_list, filename);

    SgExpression* linenrTransformed = buildString("x%%x");
    appendExpression(arg_list, linenrTransformed);

    ROSE_ASSERT(roseCreateVariable);
    string symbolName2 = roseCreateVariable->get_name().str();
    SgFunctionRefExp* memRef_r = buildFunctionRefExp(  roseCreateVariable);

    return buildFunctionCallExp(memRef_r, arg_list);
}

// convenience function
SgExprStatement*
RtedTransformation::buildVariableCreateCallStmt( SgThisExp* exp, bool forceinit) {
    SgFunctionCallExp* fn_call = buildVariableCreateCallExpr( exp, forceinit );
    if (fn_call==NULL) return NULL;
    return buildVariableCreateCallStmt( fn_call );
}

// convenience function
SgExprStatement*
RtedTransformation::buildVariableCreateCallStmt(
        SgInitializedName* initName, SgStatement* stmt, bool forceinit) {

    SgFunctionCallExp* fn_call 
        = buildVariableCreateCallExpr( initName, stmt, forceinit );
    if (fn_call==NULL) return NULL;
    return buildVariableCreateCallStmt( fn_call );
}

SgExprStatement*
RtedTransformation::buildVariableCreateCallStmt( SgFunctionCallExp* funcCallExp ) {
    SgExprStatement* exprStmt = buildExprStatement(funcCallExp);
    string empty_comment = "";
    attachComment(exprStmt,empty_comment,PreprocessingInfo::before);
    string comment = "RS : Create Variable, paramaters : (name, mangl_name, type, basetype, address, sizeof, initialized, fileOpen, classname, filename, linenr, linenrTransformed)";
    attachComment(exprStmt,comment,PreprocessingInfo::before);

    return exprStmt;
}


SgExpression*
RtedTransformation::buildVariableInitCallExpr(
        SgInitializedName* initName,
        SgVarRefExp* varRefE,
        SgStatement* stmt,
        bool ismalloc) {

  // build the function call : runtimeSystem-->createArray(params); ---------------------------
  SgExprListExp* arg_list = buildExprListExp();
  //SgExpression* simplename = buildString(initName->get_name().str());
  //appendExpression(arg_list, simplename);
  //SgExpression* callName = buildString(initName->get_mangled_name().str());
  //appendExpression(arg_list, callName);

  // with
  //    arr[ ix ] = value;
  // we want the type of (arr[ ix ]), not arr, as that is the type being
  // written
  SgExpression* exp = getExprBelowAssignment( varRefE );

  appendTypeInformation(
        NULL,
        exp -> get_type(),
        arg_list
  );
  appendClassName( arg_list, exp -> get_type() );
  appendAddressAndSize(initName, exp, arg_list,0);


  SgIntVal* ismallocV = buildIntVal(0);
  if (ismalloc)
    ismallocV = buildIntVal(1);
  appendExpression(arg_list, ismallocV);

  // with
  //    int* p;
  // this is a pointer change
  //    p = (int*) malloc(sizeof(int));
  // but this is not
  //    *p = 10;
  int is_pointer_change = 
        isSgExprStatement( stmt ) 
        && isSgPointerType( isSgExprStatement( stmt ) 
                -> get_expression() -> get_type() );
  appendExpression( arg_list, buildIntVal( is_pointer_change ));

  SgExpression* filename = buildString(stmt->get_file_info()->get_filename());
  SgExpression* linenr = buildString(RoseBin_support::ToString(stmt->get_file_info()->get_line()));
  appendExpression(arg_list, filename);
  appendExpression(arg_list, linenr);
  SgExpression* linenrTransformed = buildString("x%%x");
  appendExpression(arg_list, linenrTransformed);

  //appendExpression(arg_list, buildString(removeSpecialChar(stmt->unparseToString())));

  ROSE_ASSERT(roseInitVariable);
  string symbolName2 = roseInitVariable->get_name().str();
  SgFunctionRefExp* memRef_r = buildFunctionRefExp(  roseInitVariable);
  return buildFunctionCallExp(memRef_r, arg_list);
}

void RtedTransformation::insertInitializeVariable(SgInitializedName* initName,
						  SgVarRefExp* varRefE,
						  bool ismalloc
						  ) {
	  SgStatement* stmt=NULL;
	if (varRefE->get_parent()) // we created a verRef for AssignInitializers which do not have a parent
		stmt = getSurroundingStatement(varRefE);
	else
		stmt = getSurroundingStatement(initName);

  // make sure there is no extern in front of stmt

  if (isSgStatement(stmt)) {
    SgScopeStatement* scope = stmt->get_scope();
    string name = initName->get_mangled_name().str();
    cerr << "          ... running insertInitializeVariable :  "<<name<< "   scope: " << scope->class_name() << endl;

    ROSE_ASSERT(scope);
    // what if there is an array creation within a ClassDefinition
    if ( isSgClassDefinition(scope)) {
      // new stmt = the classdef scope
      SgClassDeclaration* decl = isSgClassDeclaration(scope->get_parent());
      ROSE_ASSERT(decl);
      stmt = isSgVariableDeclaration(decl->get_parent());
      if (!stmt) {
	cerr << " Error . stmt is unknown : " << decl->get_parent()->class_name() << endl;
	ROSE_ASSERT( false );
      }
      scope = scope->get_scope();
      // We want to insert the stmt before this classdefinition, if its still in a valid block
      cerr <<" ------->....... Found ClassDefinition Scope. New Scope is : " << scope->class_name() << "  stmt:" << stmt->class_name() <<"\n\n\n\n"<<endl;
    }
    // what is there is an array creation in a global scope
    else if (isSgGlobal(scope)) {
      cerr <<" ------->....... RuntimeInstrumentation :: WARNING - Scope not handled!!! : " << name << " : " << scope->class_name() << "\n\n\n\n" <<endl;
      // We need to add this new statement to the beginning of main
      // get the first statement in main as stmt
      stmt = mainFirst;
      scope=stmt->get_scope();
    }
    if( isSgForInitStatement( stmt -> get_parent() )) {
        // we have to handle for statements separately, because of parsing
        // issues introduced by variable declarations in the for loop's
        // init statement
        prependPseudoForInitializerExpression(
            buildVariableInitCallExpr( initName, varRefE, stmt, ismalloc ),
            isSgForStatement( stmt -> get_parent() -> get_parent() )
        );
    } else if ( isNormalScope( scope )) {

      SgExpression* funcCallExp 
          = buildVariableInitCallExpr( initName, varRefE, stmt, ismalloc );
      SgExprStatement* exprStmt = buildExprStatement(funcCallExp);
      string empty_comment = "";
      attachComment(exprStmt,empty_comment,PreprocessingInfo::before);
      string comment = "RS : Init Variable, paramaters : (name, mangl_name, tpye, basetype, class_name, address, size, ismalloc, is_pointer_change, filename, line, linenrTransformed, error line)";
      attachComment(exprStmt,comment,PreprocessingInfo::before);

      // insert new stmt (exprStmt) before (old) stmt
      insertStatementAfter(isSgStatement(stmt), exprStmt);
    } // basic block
    else if (isSgNamespaceDefinitionStatement(scope)) {
      cerr <<" ------------> RuntimeInstrumentation :: WARNING - Scope not handled!!! : " << name << " : " << scope->class_name() << "\n\n\n\n"<<endl;
    } else {
      cerr
	<< " -----------> RuntimeInstrumentation :: Surrounding Block is not Block! : "
	<< name << " : " << scope->class_name() << "  - " << stmt->unparseToString() << endl;
      ROSE_ASSERT(false);
    }
  } else {
    cerr
      << "RuntimeInstrumentation :: Surrounding Statement could not be found! "
      << stmt->class_name() << "  " << stmt->unparseToString() << endl;
    ROSE_ASSERT(false);
  }


}



void RtedTransformation::insertAccessVariable(SgVarRefExp* varRefE,
			SgExpression* derefExp
						  ) {
	SgStatement* stmt = getSurroundingStatement(varRefE);
  // make sure there is no extern in front of stmt
  SgInitializedName* initName = varRefE->get_symbol()->get_declaration();

  SgDotExp* parent_dot = isSgDotExp( varRefE -> get_parent() );
  if(	parent_dot
		&& parent_dot -> get_lhs_operand() == varRefE ) {
	  //	x = s.y
	  // does not need a var ref to y, only to s
	  return;
  }

  if (isSgStatement(stmt)) {
    SgScopeStatement* scope = stmt->get_scope();
    string name = initName->get_mangled_name().str();
    cerr << "          ... running insertAccessVariable :  "<<name<< "   scope: " << scope->class_name() << endl;

    ROSE_ASSERT(scope);
    // what if there is an array creation within a ClassDefinition
    if ( isSgClassDefinition(scope)) {
      // new stmt = the classdef scope
      SgClassDeclaration* decl = isSgClassDeclaration(scope->get_parent());
      ROSE_ASSERT(decl);
      stmt = isSgVariableDeclaration(decl->get_parent());
      if (!stmt) {
	cerr << " Error . stmt is unknown : " << decl->get_parent()->class_name() << endl;
	ROSE_ASSERT( false );
      }
      scope = scope->get_scope();
      // We want to insert the stmt before this classdefinition, if its still in a valid block
      cerr <<" ------->....... Found ClassDefinition Scope. New Scope is : " << scope->class_name() << "  stmt:" << stmt->class_name() <<"\n\n\n\n"<<endl;
    }
    // what is there is an array creation in a global scope
    else if (isSgGlobal(scope)) {
      cerr <<" ------->....... RuntimeInstrumentation :: WARNING - Scope not handled!!! : " << name << " : " << scope->class_name() << "\n\n\n\n" <<endl;
      // We need to add this new statement to the beginning of main
      // get the first statement in main as stmt
      stmt = mainFirst;
      scope=stmt->get_scope();
    }
    if ( isNormalScope( scope )) {
      // build the function call : runtimeSystem-->createArray(params); ---------------------------
      SgExprListExp* arg_list = buildExprListExp();
      //SgExpression* simplename = buildString(initName->get_name().str());
      //appendExpression(arg_list, simplename);
      //SgExpression* callName = buildString(initName->get_mangled_name().str());
      //appendExpression(arg_list, callName);


      int read_write_mask = Read;
      SgExpression* accessed_exp = varRefE;
      SgExpression* write_location_exp = varRefE;
      if( derefExp ) {
		  SgPointerDerefExp* deref_op = isSgPointerDerefExp( derefExp );
		  SgArrowExp* arrow_op = isSgArrowExp( derefExp );
		  ROSE_ASSERT( deref_op || arrow_op );

		  if( arrow_op ) {
              // with
              //    p -> b = 2
              // we need to be able to read
              //    *p
			  accessed_exp =  buildPointerDerefExp( arrow_op -> get_lhs_operand() );
		  } else {
			  // consider
			  //    int *p;
			  //    *p = 24601;
			  //  It is necessary that &p, sizeof(p) is readable, but not 
			  //  &(*p), sizeof(*p).
			  if( isUsedAsLvalue( derefExp )) {
				  accessed_exp = deref_op -> get_operand();
				  write_location_exp = deref_op;
				  read_write_mask |= Write;
			  } else
				  accessed_exp = deref_op;
		  }
      }
      appendAddressAndSize(initName, accessed_exp, arg_list,2);
      appendAddressAndSize(initName, write_location_exp, arg_list,2);
      appendExpression( arg_list, buildIntVal( read_write_mask ));

      SgExpression* filename = buildString(stmt->get_file_info()->get_filename());
      SgExpression* linenr = buildString(RoseBin_support::ToString(stmt->get_file_info()->get_line()));
      appendExpression(arg_list, filename);
      appendExpression(arg_list, linenr);

      SgExpression* linenrTransformed = buildString("x%%x");
      appendExpression(arg_list, linenrTransformed);

     // appendExpression(arg_list, buildString(removeSpecialChar(stmt->unparseToString())));

      ROSE_ASSERT(roseAccessVariable);
      string symbolName2 = roseAccessVariable->get_name().str();
      //cerr << " >>>>>>>> Symbol Member: " << symbolName2 << endl;
      SgFunctionRefExp* memRef_r = buildFunctionRefExp(  roseAccessVariable);
      SgFunctionCallExp* funcCallExp = buildFunctionCallExp(memRef_r,
							    arg_list);
      SgExprStatement* exprStmt = buildExprStatement(funcCallExp);
      // insert new stmt (exprStmt) before (old) stmt
      insertStatementBefore(isSgStatement(stmt), exprStmt);
      string empty_comment = "";
      attachComment(exprStmt,empty_comment,PreprocessingInfo::before);
      string comment = "RS : Access Variable, paramaters : (name, mangl_name, address, sizeof(type), filename, line, line transformed, error Str)";
      attachComment(exprStmt,comment,PreprocessingInfo::before);
    } // basic block
    else if (isSgNamespaceDefinitionStatement(scope)) {
      cerr <<" ------------> RuntimeInstrumentation :: WARNING - Scope not handled!!! : " << name << " : " << scope->class_name() << "\n\n\n\n"<<endl;
    } else {
      cerr
	<< " -----------> RuntimeInstrumentation :: Surrounding Block is not Block! : "
	<< name << " : " << scope->class_name() << "  - " << stmt->unparseToString() << endl;
      ROSE_ASSERT(false);
    }
  } else {
    cerr
      << "RuntimeInstrumentation :: Surrounding Statement could not be found! "
      << stmt->class_name() << "  " << stmt->unparseToString() << endl;
    ROSE_ASSERT(false);
  }


}



void RtedTransformation::visit_isAssignInitializer(SgNode* n) {
  SgAssignInitializer* assign = isSgAssignInitializer(n);
  ROSE_ASSERT(assign);
  cerr << "\n\n???????????? Found assign init op : " << n->unparseToString() << endl;
  SgInitializedName* initName = NULL;
  SgNode* ancestor = n;
  while( initName == NULL && ancestor != NULL ) {
      initName = isSgInitializedName( ancestor );
      ancestor = ancestor -> get_parent();
  }
  ROSE_ASSERT(initName);

  // ---------------------------------------------
  // we now know that this variable must be initialized
  // if we have not set this variable to be initialized yet,
  // we do so
  cerr  << ">>>>>>> Setting this var to be assign initialized : " << initName->unparseToString()
	<< "  and assignInit: " << assign->unparseToString()<<  endl;
  SgStatement* stmt = getSurroundingStatement(assign);
  ROSE_ASSERT(stmt);
  SgScopeStatement* scope = stmt->get_scope();
  ROSE_ASSERT(scope);
  SgVarRefExp* varRef = buildVarRefExp(initName, scope);
  ROSE_ASSERT(varRef);

  // dont do this if the variable is global
  if (isSgGlobal(initName->get_scope())) {
  } else {
    //insertThisStatementLater[exprStmt]=stmt;
    bool ismalloc=false;
    variableIsInitialized[varRef]=std::pair<SgInitializedName*,bool>(initName,ismalloc);
    //cerr << "Inserting new statement : " << exprStmt->unparseToString() << endl;
    //cerr << "    after old statement : " << stmt->unparseToString() << endl;
  }

  // ---------------------------------------------
}
