package Chisel


import play.twirl.api.{BufferedContent, Format, Formats, MimeTypes}

import scala.collection.immutable


/**
 * Content type used in default text templates.
 */
class Cpp (elements: immutable.Seq[Cpp], text: String) extends BufferedContent[Cpp](elements, text) {
  def this(text: String) = this(Nil, Formats.safe(text))
  def this(elements: immutable.Seq[Cpp]) = this(elements, "")

  /**
   * Content type of text (`text/plain`).
   */
  def contentType = MimeTypes.TEXT
}

/**
 * Helper for utilities Txt methods.
 */
object Cpp {

  /**
   * Creates a text fragment with initial content specified.
   */
  def apply(text: String): Cpp = {
    new Cpp(text)
  }

}

/**
 * Formatter for text content.
 */
object CppFormat extends Format[Cpp] {

  /**
   * Create a text fragment.
   */
  def raw(text: String) = Cpp(text)

  /**
   * No need for a safe (escaped) text fragment.
   */
  def escape(text: String) = Cpp(text)

  /**
   * Generate an empty Txt fragment
   */
  val empty: Cpp = new Cpp("")

  /**
   * Create an Txt Fragment that holds other fragments.
   */
  def fill(elements: immutable.Seq[Cpp]): Cpp = new Cpp(elements)

}