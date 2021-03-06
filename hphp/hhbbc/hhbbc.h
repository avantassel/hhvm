/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HPHP_HHBBC_H_
#define incl_HPHP_HHBBC_H_

#include <vector>
#include <memory>
#include <string>
#include <set>

namespace HPHP { struct UnitEmitter; }
namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * This is the public API to this subsystem.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Publically-settable options that control compilation.
 */
struct Options {
  /*
   * Functions that we should assume may be used with fb_intercept.
   * Functions that aren't named in this list may be optimized with
   * the assumption they aren't intercepted, in whole_program mode.
   */
  std::set<std::string> InterceptableFunctions;

  //////////////////////////////////////////////////////////////////////

  /*
   * If true, completely remove jumps to blocks that are inferred to
   * be dead.  When false, dead blocks are replaced with Fatal
   * bytecodes.
   */
  bool RemoveDeadBlocks = false;

  /*
   * Whether to propagate constant values by replacing instructions
   * which are known to always produce a constant with instructions
   * that produce that constant.
   */
  bool ConstantProp = true;

  /*
   * If true, insert opcodes that assert inferred types, so we can
   * assume them at runtime.
   */
  bool InsertAssertions = true;
  bool InsertStackAssertions = true;

  /*
   * If true, try to filter asserts out that are "obvious" (this is a
   * code size optimization).  It can be useful to turn this option
   * off for debugging.
   *
   * Has no effect if !InsertStackAssertions.
   */
  bool FilterAssertions = true;

  /*
   * Whether to replace bytecode with less expensive bytecodes when we
   * can.  E.g. InstanceOf -> InstanceOfD or FPushFunc -> FPushFuncD.
   */
  bool StrengthReduceBC = true;

  //////////////////////////////////////////////////////////////////////
  // Flags below this line perform optimizations that intentionally
  // may have user-visible changes to program behavior.
  //////////////////////////////////////////////////////////////////////

  /*
   * If true, we'll propagate global constants and class constants
   * "unsoundly".  I.e., it is visible to the user that we may not
   * invoke autoload at places where we would have without this
   * optimization.
   */
  bool HardConstProp = true;

  /*
   * Whether or not to assume that VerifyParamType instructions must
   * throw if the parameter does not match the associated type
   * constraint.
   *
   * This changes program behavior because parameter type hint
   * validation is a recoverable fatal.  If the error handler doesn't
   * throw at runtime, the parameter may not have the inferred type.
   *
   * Currently hhvm handles this by ignoring the problem and
   * potentially segfaulting at runtime.  We want to change this to
   * require the error handler to throw in this case.  TODO(#3038582).
   */
  bool HardTypeHints = true;

  /*
   * If true, we'll try to infer the types of declared private class
   * properties.
   *
   * This is in the can-potentially-change-program-behavior section
   * because if you unserialize specially-constructed strings you
   * could create instances with private properties that don't follow
   * the inferred types.
   *
   * Currently hhvm handles this by ignoring the problem and
   * potentially segfaulting at runtime.  We want to change this to
   * fatal at unserialize time.  TODO(#2516227).
   */
  bool HardPrivatePropInference = false;
};
extern Options options;

//////////////////////////////////////////////////////////////////////

/*
 * Perform whole-program optimization on a set of UnitEmitters.
 *
 * Currently this process relies on some information from HPHPc.  It
 * expects AttrUnique/AttrPersistent have already been set up
 * correctly (but won't be wrong if they aren't set up at all), and
 * expects traits are already flattened (it might be wrong if they
 * aren't).
 */
std::vector<std::unique_ptr<UnitEmitter>>
whole_program(std::vector<std::unique_ptr<UnitEmitter>>);

/*
 * Perform single-unit optimizations.
 */
std::unique_ptr<UnitEmitter> single_unit(std::unique_ptr<UnitEmitter>);

//////////////////////////////////////////////////////////////////////

}}

#endif
