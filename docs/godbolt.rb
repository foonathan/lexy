require 'asciidoctor'
require 'asciidoctor/extensions'

class GodboltBlock < Asciidoctor::Extensions::BlockProcessor
  enable_dsl
  on_context :listing
  positional_attributes 'language'

  def process parent, reader, attrs
    lang = attrs.delete 'language'
    attrs['title'] ||= 'Example'

    example = create_example_block parent, [], attrs, content_model: :compound
    example.set_option 'collapsible'

    listing = create_listing_block example, reader.readlines, nil
    if lang
      listing.style = 'source'
      listing.set_attr 'language', lang
      listing.commit_subs
    end

    url = 'https://godbolt.org/z/' + attrs['id']
    anchor = create_anchor example, 'Try on Compiler Explorer', type: :link, target: url
    anchor.set_attr 'window', '_blank'

    example << anchor
    example << listing
    example
  end
end

Asciidoctor::Extensions.register do
  block GodboltBlock, :godbolt
end

