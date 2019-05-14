// Author: Markus Schordan, 2013.

#ifndef ASTANNOTATOR_H
#define ASTANNOTATOR_H


#include <string>

#include "DFAstAttribute.h"
#include "Labeler.h"

namespace CodeThorn {

/*! 
  * \author Markus Schordan
  * \date 2013.
 */
class AstAnnotator {
 public:
  AstAnnotator(CodeThorn::Labeler* labeler);
  AstAnnotator(CodeThorn::Labeler* labeler, VariableIdMapping* variableIdMapping);
  // annotates attributes of Type DFAstAttribute of name 'attributeName' as comment for all nodes in the AST subtree of  node 'node'.
  void annotateAstAttributesAsCommentsBeforeStatements(SgNode* node, std::string attributeName);
  void annotateAstAttributesAsCommentsAfterStatements(SgNode* node, std::string attributeName);
  //MS: planned: void annotateAttributeAsPragma(std::string attributeName);
 protected:
  void annotateAstAttributesAsComments(SgNode* node, std::string attributeName, PreprocessingInfo::RelativePositionType posSpecifier,std::string analysisInfoTypeDescription);
  void insertComment(std::string comment, PreprocessingInfo::RelativePositionType posSpecifier, SgStatement* node);
  CodeThorn::Labeler* _labeler;
  VariableIdMapping* _variableIdMapping;
};

} // end of namespace CodeThorn

#endif
