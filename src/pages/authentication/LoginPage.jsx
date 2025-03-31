import { useState, useRef, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import { TextField, Stack, PrimaryButton } from "@fluentui/react";
import "./Auth.css";
import { useAuth } from "../../context/AuthContext";

const LoginPage = () => {
  const { login } = useAuth(); // will be used after implementing the backend
  const [password, setPassword] = useState("");
  const [usernameOrEmail, setUsernameOrEmail] = useState("");

  const userRef = useRef();

  let navigate = useNavigate();

  const handleSubmit = (e) => {
    e.preventDefault();
    const isEmail = usernameOrEmail.includes("@");
    const requestData = {
      username: isEmail ? null : usernameOrEmail,
      email: isEmail ? usernameOrEmail : null,
      password: password,
    };

    const simulatedToken = "some-token";

    login(simulatedToken);

    navigate("/");
  };

  useEffect(() => {
    if (userRef.current) {
      userRef.current.focus();
    }
  }, []);

  return (
    <div>
      <form onSubmit={handleSubmit} className="login-container">
        <Stack tokens={{ childrenGap: 10 }}>
          <TextField
            componentRef={userRef}
            label="Username or Email"
            value={usernameOrEmail}
            onChange={(e, newValue) => setUsernameOrEmail(newValue || "")}
            required
          />
          <TextField
            label="Password"
            type="password"
            value={password}
            onChange={(e, newValue) => setPassword(newValue || "")}
            required
          />
          <PrimaryButton
            text="Login"
            type="submit"
            className="custom-primary-button"
          />
        </Stack>
      </form>
    </div>
  );
};

export default LoginPage;
