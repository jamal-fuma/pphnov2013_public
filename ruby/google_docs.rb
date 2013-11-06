#!/usr/bin/env ruby

require 'rubygems'
require 'selenium-webdriver'

presentation = "https://docs.google.com/a/pearson.com/presentation/d/1_2FjgNNCDQsrPOJBxDl6Ht1UvsR1d3MoBbdmtx8gpjs/edit#slide=id.p"

class GoogleDocs
  def initialize() 
    # default_profile = Selenium::WebDriver::Firefox::Profile.from_name "default"
    # default_profile.native_events = true

    @browser = Selenium::WebDriver.for :firefox #, :profile => default_profile
    @wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @handle = @browser.window_handle
    puts "handle => #{@handle}"
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
    @wait.until {
      @browser.title.downcase.start_with? "pphnov13"
    }
  
    @browser.action.key_down(:control).send_keys(:f5).perform
    @browser.window_handles.each { |h|
      puts "I have handle => #{h}"
      if h != @handle 
        puts "switching to #{h}"
        @browser.switch_to.window(h)
        break
      end
    }

  end

  def next()
    @browser.action.send_keys(:arrow_right).perform
    puts "next"
  end

  def prev()
    @browser.action.send_keys(:arrow_left).perform
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

# Login (to Pearson)
g.login('userid','password')

# start the slideshow
g.slideshow

# Move forward and backwards through the presentation 
g.next()
g.next()
g.prev()
g.prev()
g.next()
g.next()
g.next()

# We're all done, close
# g.close

puts "All done"
