// clox-basic - C Language Implementation of jlox from Crafting Interpreters.
//
// Copyright (C) 2022 Amritpal Singh
//
// This file is part of clox-basic.
//
// clox-basic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3
// of the License, or (at your option) any later version.
//
// clox-basic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// clox-basic. If not, see <https://www.gnu.org/licenses/>.

#include "parser.h"
#include "token.h"

/* initialises a Binary expression
 * Params :
 * @left : expression to the left of Operator
 * @Operator : Operator token
 * @right : expression to the right of Operator
 * @visitor : pointer to a function taking a struct Binary_e returns void
 * this function is part of visitor pattern. Call the accept() method to perform
 * whatever operation on the same object type
 * Ret :
 * @struct Binary_e : newly initialised instance of the structure
 */
struct Binary_e
init_binary_expr(Expr* left,
                 Token* Operator,
                 Expr* right,
                 void (*visitor)(struct Binary_e*))
{
    return (struct Binary_e){
        .left = left, .Operator = Operator, .right = right, .accept = visitor
    };
}

/* initialises a Grouping expression
 * Params :
 * @expression : the expression inside a (group)
 * @visitor : pointer to a function taking a struct Binary_e returns void
 * this function is part of visitor pattern. Call the accept() method to perform
 * whatever operation on the same object type
 * Ret :
 * @struct Grouping_e : newly initialised instance of the structure
 */
struct Grouping_e
init_group_expr(Expr* expression, void (*visitor)(struct Grouping_e*))
{
    return (struct Grouping_e){ .expression = expression, .accept = visitor };
}

/* initialises a Unary expression
 * Params :
 * @Operator : the operator token
 * @right : the expression to which the unary operator is to be applied to
 * @visitor : pointer to a function taking a struct Binary_e returns void
 * this function is part of visitor pattern. Call the accept() method to perform
 * whatever operation on the same object type
 * Ret :
 * @struct Unary_e : newly initialised instance of the structure
 */
struct Unary_e
init_unary_expr(Token* Operator, Expr* right, void (*visitor)(struct Unary_e*))
{
    return (
      struct Unary_e){ .Operator = Operator, .right = right, .accept = visitor };
}

/* initialises a literal expression
 * Params :
 * @type : TOKEN_TYPE enumeration
 * @literal: a literal of type union LITERAL VALUE
 * @visitor : pointer to a function taking a struct Binary_e returns void
 * this function is part of visitor pattern. Call the accept() method to perform
 * whatever operation on the same object type
 * Ret :
 * @struct Literal_e : newly initialised instance of the structure
 */
struct Literal_e
init_literal_expression(enum TOKEN_TYPE type,
                        void* literal,
                        void (*visitor)(struct Literal_e*))
{
    return (struct Literal_e){ .type = type, .value = literal, .accept = visitor };
}

/* initialises an expression
 * Params :
 * @type : expression type enumeration
 * @Holder : a holder union that stores only the data for the type of expression
 * @visitor : a pointer to a function taking an Expr and returning void
 * this function is part of visitor pattern. Call the accept() method to perform
 * whatever operation on the same object type */
Expr
init_expression(enum EXPR_TYPES type, void* holder, void (*visitor)(Expr*))
{
    return (Expr){ .type = type, .expression_structure = holder, .accept = visitor };
}