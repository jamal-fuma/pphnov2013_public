#!/usr/bin/env ruby

require 'rubygems'
require 'capybara'
require 'capybara/dsl'

Capybara.run_server = false
Capybara.current_driver = :selenium
Capybara.app_host = 'http://www.google.com'

class Actions
    include Capybara::DSL
    def test(site)
      visit(site)
    end
end

t = Actions.new
t.test('https://docs.google.com/a/pearson.com/presentation/d/1_2FjgNNCDQsrPOJBxDl6Ht1UvsR1d3MoBbdmtx8gpjs/edit#slide=id.p')
