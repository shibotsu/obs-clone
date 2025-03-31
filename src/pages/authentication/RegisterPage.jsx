import { useState, useRef, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import {
  TextField,
  Stack,
  PrimaryButton,
  DatePicker,
  Icon,
} from "@fluentui/react";
import "./Auth.css";

const RegisterPage = () => {
  const [username, setUsername] = useState("");
  const [validUsername, setValidUsername] = useState(false);

  const [password, setPassword] = useState("");
  const [validPassword, setValidPassword] = useState(false);

  const [email, setEmail] = useState("");

  const [confirmPassword, setConfirmPassword] = useState("");
  const [birthday, setBirthday] = useState(null);
  const [error, setError] = useState("");

  // regexes for checking the username/password validity
  const isValidUsername = /^[0-9A-Za-z]{4,16}$/;
  const isValidPassword = /^(?=.*?[0-9])(?=.*?[A-Za-z]).{8,32}$/;

  let navigate = useNavigate();
  const userRef = useRef();
  const emailRef = useRef();
  const passwordRef = useRef();

  const handleSubmit = (e) => {
    e.preventDefault();

    if (!password || !confirmPassword || !username || !email || !birthday) {
      setError("All fields are required.");
      return;
    }

    if (password !== confirmPassword) {
      setError("Passwords do not match.");
      setPassword("");
      setConfirmPassword("");
      if (passwordRef.current) {
        passwordRef.current.focus();
      }
      return;
    }

    if (!email.includes("@")) {
      setError("Please enter a valid email.");
      if (emailRef.current) {
        emailRef.current.focus();
      }

      return;
    }

    navigate("/login");
  };

  const onDateChange = (date) => {
    setBirthday(date);
  };

  // on loading the page focus on the username/email field
  useEffect(() => {
    if (userRef.current) {
      userRef.current.focus();
    }
  }, []);

  // check if the username is valid
  useEffect(() => {
    const result = isValidUsername.test(username);
    setValidUsername(result);
  }, [username]);

  useEffect(() => {
    const result = isValidPassword.test(password);
    setValidPassword(result);
  }, [password]);

  const renderIcon = (isValid, value) => {
    return isValid && value.length > 0 ? (
      <Icon
        iconName="CheckmarkFilled"
        style={{ color: "green", marginLeft: 8 }}
      />
    ) : (
      <Icon
        iconName="ErrorCircleRegular"
        style={{ color: "red", marginLeft: 8 }}
      />
    );
  };

  const today = new Date();

  return (
    <div>
      <form onSubmit={handleSubmit} className="login-container">
        <Stack tokens={{ childrenGap: 10 }}>
          <TextField
            componentRef={userRef}
            className="textfield"
            label="Username"
            value={username}
            onChange={(e, newValue) => setUsername(newValue || "")}
            autoComplete="off"
            aria-invalid={validUsername ? "false" : "true"}
            aria-describedby="uidnote"
            errorMessage={
              username && !validUsername
                ? "Username must be 4-16 alphanumerical characters."
                : ""
            }
            onRenderSuffix={() => renderIcon(validUsername, username)}
            required
          />
          <TextField
            componentRef={emailRef}
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
            componentRef={passwordRef}
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
            type="submit"
            className="custom-primary-button"
          />
        </Stack>
      </form>
    </div>
  );
};

export default RegisterPage;
