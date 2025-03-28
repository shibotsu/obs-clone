import { useState } from "react";
import { useNavigate } from "react-router-dom";
import { DatePicker } from "@fluentui/react";
import { TextField, Stack, PrimaryButton } from "@fluentui/react";
import "./Auth.css";

const RegisterPage = () => {
  const [password, setPassword] = useState("");
  const [confirmPassword, setConfirmPassword] = useState("");
  const [username, setUsername] = useState("");
  const [email, setEmail] = useState("");
  const [birthday, setBirthday] = useState(null);
  const [error, setError] = useState("");

  let navigate = useNavigate();

  const handleSubmit = () => {
    if (!password || !confirmPassword || !username || !email || !birthday) {
      setError("All fields are required.");
      return;
    }

    if (password !== confirmPassword) {
      setError("Passwords do not match.");
      setPassword("");
      setConfirmPassword("");
      return;
    }

    navigate("/login");
  };

  const onDateChange = (date) => {
    setBirthday(date);
  };

  const today = new Date();

  return (
    <div className="login-container">
      <Stack tokens={{ childrenGap: 10 }}>
        <TextField
          className="textfield"
          label="Username"
          value={username}
          onChange={(e, newValue) => setUsername(newValue || "")}
          required
        />
        <TextField
          className="textfield"
          label="Email"
          value={email}
          onChange={(e, newValue) => setEmail(newValue || "")}
          required
        />
        <TextField
          className="textfield"
          label="Password"
          type="password"
          value={password}
          onChange={(e, newValue) => setPassword(newValue || "")}
          required
        />
        <TextField
          className="textfield"
          label="Confirm Password"
          type="password"
          value={confirmPassword}
          onChange={(e, newValue) => setConfirmPassword(newValue || "")}
          required
        />
        <DatePicker
          className="date-picker"
          label="Enter your birthday"
          value={birthday}
          onSelectDate={onDateChange}
          maxDate={today}
          required
        />
        {error && <p style={{ color: "red" }}>{error}</p>}
        <PrimaryButton
          text="Sign Up"
          onClick={handleSubmit}
          className="custom-primary-button"
        />
      </Stack>
    </div>
  );
};

export default RegisterPage;
