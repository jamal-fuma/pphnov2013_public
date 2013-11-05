#!/usr/bin/env ruby

require 'rubygems'
require 'selenium-webdriver'

presentation = "https://docs.google.com/a/pearson.com/presentation/d/1_2FjgNNCDQsrPOJBxDl6Ht1UvsR1d3MoBbdmtx8gpjs/edit#slide=id.p"

class GoogleDocs
  def initialize() 
    default_profile = Selenium::WebDriver::Firefox::Profile.from_name "default"
    default_profile.native_events = true

    @browser = Selenium::WebDriver.for :firefox, :profile => default_profile
    @wait = Selenium::WebDriver::Wait.new(:timeout => 15)
  end

  def connect(url)
    @browser.get url

  end

  def login(user,password)
    @wait.until {
      @browser.title
    }

    if @browser.title.downcase.start_with? "sign in" 
      name = @wait.until {
        element = @browser.find_element(:xpath,"//*[@id = 'ctl00_ContentPlaceHolder1_PremiumDevice1_UsernameTextBox']")
        element if element.displayed?
      }
      name.send_keys(user)

      passwd = @wait.until {
        element = @browser.find_element(:xpath,"//*[@id = 'ctl00_ContentPlaceHolder1_PremiumDevice1_PasswordTextBox']")
        element if element.displayed?
      }
      passwd.send_keys(password)

      submit = @wait.until {
        element = @browser.find_element(:xpath,"//*[@id = 'ctl00_ContentPlaceHolder1_PremiumDevice1_SubmitButton']")
        element if element.displayed?
      }
      submit.click()
    end
  end

  def slideshow()
    @browser.action.key_down(:control).send_keys(:f5).perform
    @browser.switch_to.default_content
    @browser.execute_script('window.focus();')
  end

  def next()
    next_but = @wait.until {
        element = @browser.find_element(:xpath,"/*[@role='button' and contains(@title,'Next')]")
        element if element.displayed?
      }
      puts next_but
      next_but.click()
    puts "next"
  end

  def prev()
    current_url = @browser.current_url
    @browser.action.send_keys(:page_down).perform
    @wait.until {
      @browser.current_url != current_url
    }
    puts "prev"
  end

  def close()
    @browser.close()
    @browser.quit()
  end
end

# start presentation
g = GoogleDocs.new
g.connect('https://docs.google.com/a/pearson.com/presentation/d/1_2FjgNNCDQsrPOJBxDl6Ht1UvsR1d3MoBbdmtx8gpjs/edit#slide=id.p')
g.login('my_userid','my_password')
g.slideshow

g.next()
# g.next()
# g.prev()
# g.prev()
# g.next()
# g.next()
# g.next()

# g.close

puts "All done"
