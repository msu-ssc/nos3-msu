require 'cosmos'
require 'cosmos/script'
require 'sample_radio_lib.rb'

class SAMPLE_RADIO_Functional_Test < Cosmos::Test
  def setup
    safe_sample_radio()
  end

  def test_application
      start("tests/sample_radio_app_test.rb")
  end

  def test_device
    start("tests/sample_radio_device_test.rb")
  end

  def teardown
    safe_sample_radio()
  end
end

class SAMPLE_RADIO_Automated_Scenario_Test < Cosmos::Test
  def setup 
    safe_sample_radio()
  end

  def test_AST
      start("tests/sample_radio_ast_test.rb")
  end

  def teardown
    safe_sample_radio()
  end
end

class Sample_radio_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('SAMPLE_RADIO_Functional_Test')
      add_test('SAMPLE_RADIO_Automated_Scenario_Test')
  end

  def setup
    safe_sample_radio()
  end
  
  def teardown
    safe_sample_radio()
  end
end
