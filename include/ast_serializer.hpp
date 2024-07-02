#include "ast.hpp"
#include "ast_visitor.hpp"
#include <sstream>



struct ASTSerializer;

// This struct uses RAII to provide scope-based indentation for the serializer.
// Pretty Kool!
struct Indenter {
	ASTSerializer *serializer;
	Indenter(ASTSerializer *serializer);
	~Indenter();
};

struct ASTSerializer : ASTVisitor {
	ASTSerializer(int indent_level = 0, int indent_size = 2);
	int indent_level;
	int indent_size;
	
	void Write(const std::string &str) {
		stream << string(indent_level, ' ') + str << "\n";
	}
	std::string Indent() {
		return std::string(indent_level, ' ');
	}
	
	std::stringstream stream;
	void visit(ASTNode *_) override;
	void visit(Executable *_) override;
	void visit(Statement *_) override;
	void visit(Program *program) override;
	void visit(Expression *_) override;
	void visit(Declaration *_) override;
	void visit(Operand *operand) override;
	void visit(Identifier *identifier) override;
	void visit(Arguments *arguments) override;
	void visit(TupleInitializer *tupleInitializer) override;
	void visit(Property *property) override;
	void visit(Parameters *parameters) override;
	void visit(Continue *_) override;
	void visit(Break *_) override;
	void visit(Return *ret) override;
	void visit(Delete *del) override;
	void visit(Block *block) override;
	void visit(ObjectInitializer *objInit) override;
	void visit(Call *call) override;
	void visit(If *ifStmt) override;
	void visit(Else *elseStmt) override;
	void visit(For *forStmt) override;
	void visit(RangeBasedFor *rangeFor) override;
	void visit(Assignment *assignment) override;
	void visit(TupleDeconstruction *tupleDec) override;
	void visit(CompAssignExpr *compAssignExpr) override;
	void visit(CompoundAssignment *compoundAssign) override;
	void visit(FunctionDecl *funcDecl) override;
	void visit(Noop *_) override;
	void visit(DotExpr *dotExpr) override;
	void visit(DotAssignment *dotAssign) override;
	void visit(DotCallStmnt *dotCall) override;
	void visit(Subscript *subscript) override;
	void visit(SubscriptAssignStmnt *subscriptAssign) override;
	void visit(UnaryExpr *unaryExpr) override;
	void visit(UnaryStatement *unaryStmt) override;
	void visit(BinExpr *binExpr) override;
	void visit(Using *usingStmt) override;
	void visit(Lambda *lambda) override;
	void visit(Match *match) override;
	void visit(MatchStatement *matchStmt) override;
        void visit(MethodCall *method) override;
        void visit(Literal *literal) override;
};